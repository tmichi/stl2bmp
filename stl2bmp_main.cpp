/**
 * @file stl2bmp.cpp
 * @brief Convert STL files (binary) to 1bit BMP files (a.k.a voxelization)
 * @author Takashi Michikawa <ram-axed-0b@icloud.com> RIKEN Center for Advanced Photonics
 * @copyright MIT license (see LICENSE.txt
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
#endif

#include <vector>
#include <tuple>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <stl2bmp_version.hpp>
//#define stl2bmp_VERSION 0.1.0

#if __cplusplus >= 201703L //C++17
#include <filesystem>
namespace fs = std::filesystem;
#elif __cplusplus >= 201103L //C++11
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#error C++17 or C++14+Boost filesystem required.
#endif
bool fwrite(std::ofstream& fout) {
        return fout.good();
}
template <class Head, class... Tail>
bool fwrite(std::ofstream& fout, Head&& head, Tail&&... tail) {
        fout.write(reinterpret_cast<const char*>(&head), sizeof(Head));
        return fwrite(fout, std::forward<Tail>(tail)...);
}
int main(int argc, char **argv) {
        using triangle_t = std::tuple<Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d>;
        std::cerr << "stl2bmp v." << stl2bmp_VERSION << std::endl;
        try {
                if (argc < 2) {
                        throw std::runtime_error("Usage: " + std::string(argv[0]) + " input.stl {dpi:30}");
                }
                const fs::path input_file = argv[1];
                if (!fs::exists(input_file)) {
                        throw std::runtime_error(input_file.string() + " does not exist");
                }
                const int dpi = (argc < 3) ? 360 : std::atoi(argv[2]);
                if (dpi <= 0) {
                        throw std::runtime_error("Invalid DPI : " + std::to_string(dpi));
                }
                std::cerr<<"dpi:"<<dpi<<std::endl;
                const double pitch = 25.4 / dpi;
                const int32_t ppm = static_cast<int32_t>(std::round(1000.0 / pitch));
                
                const fs::path output_dir = input_file.stem();
                fs::create_directories(output_dir);
                if (!fs::exists(output_dir) || !fs::is_directory(output_dir)) {
                        throw std::runtime_error(output_dir.string() + " cannot be created.");
                }
                
                std::vector<triangle_t> triangles;
                std::ifstream fin(input_file.string(), std::ios::binary);
                if (!fin) {
                        throw std::runtime_error(input_file.string() + " open failed");
                } else {
                        char header[80];
                        uint32_t nt;
                        fin.read(&header[0], 80).read(reinterpret_cast<char *>(&nt), 4);
                        Eigen::Vector3f p[4];
                        uint16_t b;
                        for (uint32_t i = 0; i < nt; ++i) {
                                fin.read(reinterpret_cast<char *>(&p[0]), 4 * sizeof(float) * 3).read(reinterpret_cast<char *>(&b), sizeof(uint16_t));
                                if (!fin) {
                                        throw std::runtime_error("STL read failed");
                                }
                                triangles.emplace_back(p[1].cast<double>(), p[2].cast<double>(), p[3].cast<double>(), p[0].cast<double>());
                        }
                }
                Eigen::AlignedBox3d bbox;
                std::for_each(triangles.begin(), triangles.end(), [&bbox](triangle_t &t) { bbox.extend(std::get<0>(t)).extend(std::get<1>(t)).extend(std::get<2>(t)); });
                Eigen::Vector3d d = bbox.center() + bbox.sizes().z() * Eigen::Vector3d(0, 0, 1);
                std::for_each(triangles.begin(), triangles.end(), [&d](triangle_t &t) {
                        std::get<0>(t) -= d;
                        std::get<1>(t) -= d;
                        std::get<2>(t) -= d;
                });
                bbox.translate(-d);
                
                const float zoff = float(0.5 * bbox.sizes().z());
                const Eigen::Vector3i size = (1.0 / pitch * bbox.sizes()).array().ceil().cast<int>();
                const GLfloat black[] = {0, 0, 0, 1};
                const GLfloat white[] = {1, 1, 1, 1};
                const GLfloat pos[4]{0, 0, 0, 1};
                if (!::glfwInit()) {
                        throw std::runtime_error("glfwInit() failed");
                }
                ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // disable retina
                ::glfwWindowHint(GLFW_VISIBLE, 0);
                
                GLFWwindow *window = ::glfwCreateWindow(size.x(), size.y(), "offscreen", nullptr, nullptr);
                if (!window) {
                        throw std::runtime_error("glfwCreateWindow() failed");
                } else {
                        ::glfwMakeContextCurrent(window);
                        ::glEnable(GL_DEPTH_TEST);
                        ::glEnable(GL_LIGHT0);
                        ::glEnable(GL_LIGHTING);
                        ::glClearColor(0, 0, 0, 1);
                        ::glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
                        ::glLightfv(GL_LIGHT0, GL_POSITION, pos);
                        ::glLightfv(GL_LIGHT0, GL_AMBIENT, white);
                        ::glLightfv(GL_LIGHT0, GL_DIFFUSE, black);
                        ::glLightfv(GL_LIGHT0, GL_SPECULAR, black);
                        ::glLightfv(GL_LIGHT0, GL_EMISSION, black);
                        std::cerr << "\r" << std::setw(5) << 0 << "/" << size.z();
                        for (int z = 0; z < size.z(); ++z) {
                                ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                                ::glMatrixMode(GL_PROJECTION);
                                ::glLoadIdentity();
                                ::glOrtho(bbox.min().x(), bbox.max().x(), bbox.min().y(), bbox.max().y(), zoff + (z + 0.5) * pitch, zoff + (size.z() + 0.5) * pitch);
                                ::glMatrixMode(GL_MODELVIEW);
                                ::glLoadIdentity();
                                ::glMaterialfv(GL_FRONT, GL_AMBIENT, black);
                                ::glMaterialfv(GL_BACK, GL_AMBIENT, white);
                                ::glBegin(GL_TRIANGLES);
                                for (auto &t: triangles) {
                                        ::glNormal3dv(std::get<3>(t).data());
                                        ::glVertex3dv(std::get<0>(t).data());
                                        ::glVertex3dv(std::get<1>(t).data());
                                        ::glVertex3dv(std::get<2>(t).data());
                                }
                                ::glEnd();
                                ::glFlush();
                                
                                std::vector<uint8_t> buffer(uint32_t(size.head<2>().prod() * 4), 0x00);
                                ::glReadPixels(0, 0, size.x(), size.y(), GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
                                
                                std::stringstream ss;
                                ss << output_dir.string() << "/image" << std::setw(5) << std::setfill('0') << size.z() -1 -z << ".bmp";
                                std::ofstream fout(ss.str(), std::ios::binary);
                                if (!fout) {
                                        throw std::runtime_error(ss.str() + " cannot be open");
                                } else {
                                        const uint32_t bytePerLine = static_cast<uint32_t> ((((size.x() + 7) / 8 + 3) / 4) * 4);
                                        const uint32_t imageSize = uint32_t(size.y()) * bytePerLine;
                                        const uint32_t numPalettes = 2;
                                        fwrite(fout, uint16_t(0x4D42), uint32_t(62u + imageSize), uint16_t(0), uint16_t(0), uint32_t(14u + 40u + numPalettes * 4u));
                                        fwrite(fout, uint32_t(40), size.x(),size.y(), uint16_t(1), uint16_t(1), uint32_t(0), imageSize, ppm, ppm, numPalettes, uint32_t(0u));
                                        fwrite(fout, uint32_t(0x00000000), uint32_t(0x00ffffff)); //Palette 1
                                        std::vector<uint8_t> line(bytePerLine, 0x00);
                                        size_t i = 0;
                                        for (uint16_t y = 0; y < size.y(); y++) {
                                                std::fill(line.begin(), line.end(), 0x00);
                                                for (uint16_t x = 0; x < size.x(); ++x, i += 4) {
                                                        if (buffer[i] > 0) {
                                                                line[x / 8] |= 0x01 << (7 - x % 8);
                                                        }
                                                }
                                                fout.write(reinterpret_cast<char *>(line.data()), bytePerLine);
                                        }
                                }
                                std::cerr << "\r" << std::setw(5) << z+1 << "/" << size.z();
                        }
                        std::cerr << std::endl << "Images saved to " << fs::absolute(output_dir) << "." << std::endl;
                }
                ::glfwTerminate();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error." << std::endl;
        }
        return EXIT_SUCCESS;
}
