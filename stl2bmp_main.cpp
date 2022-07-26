/**
 * @file stl2bmp.cpp
 * @brief Convert STL files (binary) to 1bit BMP files (a.k.a voxelization)
 * @author Takashi Michikawa <peppery-eternal.0b@icloud.com> RIKEN Center for Advanced Photonics
 * MIT License (See LICENSE.txt).
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
#endif

#include <fstream>
#include <array>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <stl2bmp_version.hpp>
#include <filesystem>

namespace fs = std::filesystem;

bool fwrite(std::ofstream &fout) {
        return fout.good();
}
template <class Head, class... Tail>
bool fwrite(std::ofstream& fout, Head&& head, Tail&&... tail) {
        fout.write(reinterpret_cast<const char*>(&head), sizeof(Head));
        return fwrite(fout, std::forward<Tail>(tail)...);
}

int main(int argc, char **argv) {
        std::cerr << "stl2bmp v." << stl2bmp_VERSION << std::endl;
        try {
                if (argc < 2) {
                        throw std::runtime_error("Usage: " + std::string(argv[0]) + " input.stl {dpi:30}");
                }
                const fs::path input_file{argv[1]};
                if (!fs::exists(input_file)) {
                        throw std::runtime_error(input_file.string() + " does not exist");
                }
                const fs::path output_dir = input_file.stem();
                const int dpi = (argc < 3) ? 360 : std::stoi(argv[2]);
                if (dpi <= 0) {
                        throw std::runtime_error("Invalid DPI : " + std::to_string(dpi));
                }
                const double pitch = 25.4 / dpi;
                const auto ppm = static_cast<int32_t>(std::round(1000.0 / pitch));
                fs::create_directories(output_dir);
                if (!fs::exists(output_dir) || !fs::is_directory(output_dir)) {
                        throw std::runtime_error(output_dir.string() + " cannot be created.");
                }
                Eigen::MatrixXf vertices, normals;
                std::ifstream fin(input_file.string(), std::ios::binary);
                if (!fin) {
                        throw std::runtime_error(input_file.string() + " open failed");
                } else {
                        std::array<char, 80> header{};
                        uint32_t nt;
                        fin.read(header.data(), 80).read(reinterpret_cast<char *>(&nt), 4);
                        vertices.resize(3, nt * 3);
                        normals.resize(3, nt);
                        for (uint32_t i = 0; i < nt; ++i) {
                                fin.read(reinterpret_cast<char *>(normals.col(i).data()), 12);
                                fin.read(reinterpret_cast<char *>(vertices.col(i * 3).data()), 12);
                                fin.read(reinterpret_cast<char *>(vertices.col(i * 3 + 1).data()), 12);
                                fin.read(reinterpret_cast<char *>(vertices.col(i * 3 + 2).data()), 12);
                                fin.seekg(2, std::ios::cur);
                                if (!fin) {
                                        throw std::runtime_error("STL read failed");
                                }
                        }
                }
                auto bmin = vertices.rowwise().minCoeff();
                auto bmax = vertices.rowwise().maxCoeff();
                auto sizes = bmax - bmin;
                vertices.colwise() -= 0.5 * (bmin + bmax) + Eigen::Vector3f(0, 0, sizes.z());
                const float zoff = 0.5f * sizes.z();
                const Eigen::Vector3i size = (1.0 / pitch * sizes).array().ceil().cast<int>();
                const GLfloat black[] = {0, 0, 0, 1};
                const GLfloat white[] = {1, 1, 1, 1};
                if (!::glfwInit()) {
                        throw std::runtime_error("glfwInit() failed");
                }
                ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // disable retina
                ::glfwWindowHint(GLFW_VISIBLE, 0);
                ::GLFWwindow *window = ::glfwCreateWindow(size.x(), size.y(), "offscreen", nullptr, nullptr);
                if (!window) {
                        throw std::runtime_error("glfwCreateWindow() failed");
                }
                ::glfwMakeContextCurrent(window);
                ::glClearColor(0, 0, 0, 1);
                ::glEnable(GL_DEPTH_TEST);
                ::glEnable(GL_LIGHTING);
                ::glEnable(GL_LIGHT1);
                ::glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
                ::glLightfv(GL_LIGHT1, GL_AMBIENT, white);
                ::glMaterialfv(GL_FRONT, GL_AMBIENT, black);
                ::glMaterialfv(GL_BACK, GL_AMBIENT, white);
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
        
                //const auto bpl = uint32_t((size.x() + 3 ) / 4 * 4);
                std::vector<uint8_t> line(static_cast<uint32_t> ((((size.x() + 7) / 8 + 3) / 4) * 4), 0x00);
                for (int z = 0; z < size.z(); ++z) {
                        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        ::glMatrixMode(GL_PROJECTION);
                        ::glLoadIdentity();
                        ::glOrtho(bmin.x(), bmax.x(), bmin.y(), bmax.y(), zoff + (z + 0.5) * pitch, zoff + (size.z() + 0.5) * pitch);
                        ::glMatrixMode(GL_MODELVIEW);
                        ::glLoadIdentity();
                        ::glCallList(1);
                        ::glFlush();
                        std::vector<uint8_t> buffer(static_cast<uint32_t>(4 * size.x() * size.y()), 0x00); //color buffer
                        ::glReadPixels(0, 0, size.x(), size.y(), GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
                        std::stringstream ss;
                        ss << output_dir.string() << "/image" << std::setw(5) << std::setfill('0') << size.z() - 1 - z << ".bmp";
                        std::ofstream fout(ss.str(), std::ios::binary);
                        if (!fout) {
                                throw std::runtime_error(ss.str() + " cannot be open");
                        } else {
                                const uint32_t headerSize = 14u + 40u + 2 * 4u;
                                const uint32_t imageSize = uint32_t(size.y()) * uint32_t(line.size());
                                fwrite(fout, uint16_t(0x4D42), headerSize + imageSize, uint16_t(0), uint16_t(0), headerSize);
                                fwrite(fout, uint32_t(40), size.x(), size.y(), uint16_t(1), uint16_t(1), uint32_t(0), imageSize, ppm, ppm, uint32_t(2), uint32_t(0u));
                                fwrite(fout, uint32_t(0x00000000), uint32_t(0x00ffffff)); //Palette 1
                                for (uint16_t y = 0; y < size.y(); y++) {
                                        std::fill(line.begin(), line.end(), 0x00);
                                        for (uint16_t x = 0; x < size.x(); ++x) {
                                                if (buffer[uint32_t(4 * (x + size.x() * y))] > 0) {
                                                        line[x / 8] |= 0x01 << (7 - x % 8);
                                                }
                                        }
                                        fout.write(reinterpret_cast<char *>(line.data()), std::streamsize(line.size()));
                                }
                        }
                        std::cerr << "\r" << std::setw(5) << z + 1 << "/" << size.z();
                }
                std::cerr << std::endl << "Images(" << size.x() << "x" << size.y() << "," << dpi << "dpi) saved to " << fs::absolute(output_dir) << "." << std::endl;
                ::glfwTerminate();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error." << std::endl;
        }
        return EXIT_SUCCESS;
}