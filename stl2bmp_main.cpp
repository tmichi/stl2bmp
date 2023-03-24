/**
 * @file stl2bmp.cpp
 * @brief Convert STL files (binary) to 1bit BMP files (a.k.a voxelization)
 * @author Takashi Michikawa <peppery-eternal.0b@icloud.com> RIKEN Center for Advanced Photonics
 * MIT License (See LICENSE.txt).
 */
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
#endif
#define MI_GL_ENABLED 1
#include "OffScreenRenderer.hpp"
#include <tuple>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <array>
#include <vector>
#include <stdexcept>
#include <Eigen/Dense>
#include <stl2bmp_version.hpp>

namespace fs = std::filesystem;

namespace mi_stl2bmp {
        bool fwrite(std::ofstream &fout) {
                return fout.good();
        }
        
        template<class Head, class... Tail>
        bool fwrite(std::ofstream &fout, Head &&head, Tail &&... tail) {
                fout.write(reinterpret_cast<const char *>(&head), sizeof(Head));
                return fwrite(fout, std::forward<Tail>(tail)...);
        }
        
        template<typename... Args>
        std::tuple<Args...> fread(std::istream &in) {
                std::tuple<Args...> result;
                std::apply([&in](Args &... v) { (in.read(reinterpret_cast<char *>(&v), sizeof(v)), ...); }, result);
                return result;
        }
        
        std::tuple<Eigen::MatrixXf, Eigen::MatrixXf> parse_stl(const std::filesystem::path &path) {
                std::ifstream fin(path, std::ios::binary);
                Eigen::MatrixXf vertices, normals;
                if (!fin) {
                        throw std::runtime_error(path.string() + " open failed");
                }
                auto [header, nt] = mi_stl2bmp::fread<std::array<char, 80>, uint32_t>(fin);
                if (std::string(header.data()).substr(0, 5) != "solid") {
                        // ASCII format
                        normals.resize(3, nt);
                        vertices.resize(3, nt * 3);
                        for (uint32_t i = 0; i < nt; ++i) {
                                auto [pbuf] = mi_stl2bmp::fread<Eigen::Matrix<float, 3, 4>>(fin);
                                normals.col(i) = pbuf.col(0);
                                vertices.block<3, 3>(0, 3 * i) = pbuf.block<3, 3>(0, 1);
                                fin.seekg(2, std::ios::cur);
                                if (!fin) {
                                        throw std::runtime_error("Binary STL read failed");
                                }
                        }
                } else {
                        // Binary format
                        fin.seekg(0);
                        nt = static_cast<uint32_t>(std::count(std::istream_iterator<std::string>(fin), std::istream_iterator<std::string>(), "facet"));
                        fin.close();
                        fin.open(path, std::ios::binary);
                        normals.resize(3, nt);
                        vertices.resize(3, nt * 3);
                        auto validate = [&fin](const std::string &valid) {
                                std::string v;
                                fin >> v;
                                if (v != valid) {
                                        throw std::runtime_error("format error " + valid);
                                }
                        };
                        auto read_triple = [&fin](auto &&v) {
                                fin >> v.x() >> v.y() >> v.z();
                        };
                        fin.seekg(0);
                        std::string buffer;
                        validate("solid");
                        for (uint32_t i = 0; i < nt; ++i) {
                                validate("facet");
                                validate("normal");
                                read_triple(normals.col(i));
                                validate("outer");
                                validate("loop");
                                validate("vertex");
                                read_triple(vertices.col(3 * i));
                                validate("vertex");
                                read_triple( vertices.col(3 * i + 1));
                                validate("vertex");
                                read_triple(vertices.col(3 * i + 2));
                                validate("endloop");
                                validate("endfacet");
                        }
                        validate("endsolid");
                }

                return {normals, vertices};
        }
}

int main(int argc, char **argv) {
        std::cerr << "stl2bmp v." << stl2bmp_VERSION << std::endl;
        constexpr std::array<GLfloat, 4> black{0, 0, 0, 1}, white{1, 1, 1, 1};
        try {
                if (argc < 2) {
                        throw std::runtime_error("Usage: " + std::string(argv[0]) + " input.stl {dpi:360}");
                }
                const fs::path input_file{argv[1]};
                if (!fs::exists(input_file)) {
                        throw std::runtime_error(input_file.string() + " file not found");
                }
                const fs::path output_dir = input_file.stem();
                const int dpi = (argc < 3) ? 360 : std::stoi(argv[2]);
                fs::create_directories(output_dir);
                if (!fs::exists(output_dir) || !fs::is_directory(output_dir)) {
                        throw std::runtime_error(output_dir.string() + " cannot be created.");
                }
                if (dpi <= 0) {
                        throw std::runtime_error("Invalid DPI : " + std::to_string(dpi));
                }
                const float pitch = 25.4f / static_cast<float>(dpi); // 1inch = 25.4 mm
                const auto ppm = static_cast<int32_t>(std::round(1000.0f / pitch));
        
                auto [normals, vertices] = mi_stl2bmp::parse_stl(input_file);
                //normalize
                Eigen::Vector3f bmin = vertices.rowwise().minCoeff();
                Eigen::Vector3f bmax = vertices.rowwise().maxCoeff();
                bmax = bmax.cwiseMax(bmin + Eigen::Vector3f{128 * pitch, 128 * pitch, 0}); // min size (128x128)
                Eigen::Vector3f sizes = bmax - bmin;
                Eigen::Vector3i size = (1.0f / pitch * sizes).array().ceil().cast<int>();
                const Eigen::Vector3f center = 0.5 * (bmin + bmax);
                vertices.colwise() -= center;
                bmin -= center;
                bmax -= center;

                if (!::glfwInit()) {
                        throw std::runtime_error("glfwInit() failed");
                }
                ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // disable retina
                ::glfwWindowHint(GLFW_VISIBLE, 0);
                ::GLFWwindow *window = ::glfwCreateWindow(128, 128, "offscreen", nullptr, nullptr);
                if (!window) {
                        throw std::runtime_error("glfwCreateWindow() failed");
                }
        
                ::glfwMakeContextCurrent(window);
        
                if (GLenum err = ::glewInit(); err != GLEW_OK) {
                        throw std::runtime_error(reinterpret_cast<const char *>(::glewGetErrorString(err)));
                }
                mi::FrameBufferObject fbo(size.x(), size.y());
                fbo.activate();
                ::glViewport(0, 0, size.x(), size.y());
                ::glClearColor(0, 0, 0, 1);
                ::glEnable(GL_DEPTH_TEST);

                ::glEnable(GL_LIGHTING);
                ::glEnable(GL_LIGHT1);
                ::glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
                ::glLightfv(GL_LIGHT1, GL_AMBIENT, white.data());
                ::glMaterialfv(GL_FRONT, GL_AMBIENT, black.data());
                ::glMaterialfv(GL_BACK, GL_AMBIENT, white.data());

                //Draw triangles.
                ::glNewList(1, GL_COMPILE);
                ::glBegin(GL_TRIANGLES);
                for (int i = 0, j = 0; i < normals.cols(); ++i, j += 3) {
                        ::glNormal3fv(normals.col(i).data());
                        ::glVertex3fv(vertices.col(j).data());
                        ::glVertex3fv(vertices.col(j + 1).data());
                        ::glVertex3fv(vertices.col(j + 2).data());
                }
                ::glEnd();
                ::glEndList();
        
                std::vector<uint8_t> line(static_cast<uint32_t> ((((size.x() + 7) / 8 + 3) / 4) * 4), 0x00);
                std::vector<float> buffer(static_cast<uint32_t>(4 * size.x() * size.y()), 0); //color buffer
                for (int z = 0; z < size.z(); ++z) {
                        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        ::glMatrixMode(GL_PROJECTION);
                        ::glLoadIdentity();
                        ::glOrtho(bmin.x(), bmax.x(), bmin.y(), bmax.y(), bmin.z() + (z + 0.5) * pitch, bmax.z() + 0.01);
        
                        ::glMatrixMode(GL_MODELVIEW);
                        ::glLoadIdentity();
                        ::glCallList(1);
                        ::glFlush();

                        fbo.getBuffer(buffer);
        
                        std::stringstream ss;
                        ss << output_dir.string() << "/image" << std::setw(5) << std::setfill('0') << size.z() - 1 - z << ".bmp";
                        std::ofstream fout(ss.str(), std::ios::binary); // write to bmp
                        if (!fout) {
                                throw std::runtime_error(ss.str() + " cannot be open");
                        } else {
                                const uint32_t headerSize = 14u + 40u + 2 * 4u;
                                const uint32_t imageSize = uint32_t(size.y()) * uint32_t(line.size());
                                mi_stl2bmp::fwrite(fout, uint16_t(0x4D42), headerSize + imageSize, uint16_t(0), uint16_t(0), headerSize);
                                mi_stl2bmp::fwrite(fout, uint32_t(40), size.x(), size.y(), uint16_t(1), uint16_t(1), uint32_t(0), imageSize, ppm, ppm, uint32_t(2), uint32_t(0u));
                                mi_stl2bmp::fwrite(fout, uint32_t(0x00000000), uint32_t(0x00ffffff)); //Palette 1
                                for (uint16_t y = 0; y < static_cast<uint16_t>(size.y()); y++) {
                                        std::fill(line.begin(), line.end(), 0x00);
                                        for (uint16_t x = 0; x < static_cast<uint16_t>(size.x()); ++x) {
                                                if (buffer[uint32_t(4 * (x + size.x() * y))] > 0) {
                                                        line[x / 8] |= 0x01 << (7 - x % 8);
                                                }
                                        }
                                        fout.write(reinterpret_cast<char *>(line.data()), std::streamsize(line.size()));
                                }
                        }
                        std::cerr << "\r" << std::setw(5) << z + 1 << "/" << size.z();
                }
                std::cerr << std::endl
                          << size.z() << " images(" << size.x() << "x" << size.y() << "," << dpi << "dpi) saved to " << fs::absolute(output_dir) << "." << std::endl;
                ::glfwTerminate();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error." << std::endl;
        }
        return EXIT_SUCCESS;
}