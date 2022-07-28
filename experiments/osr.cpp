//
// Created by Takashi Michikawa on 2022/07/28.
//

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
#endif
#define USE_MATH_DEFINES_

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <array>
#include <vector>
#include <stdexcept>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>

namespace fs = std::filesystem;

bool fwrite(std::ofstream &fout) {
        return fout.good();
}

template<class Head, class... Tail>
bool fwrite(std::ofstream &fout, Head &&head, Tail &&... tail) {
        fout.write(reinterpret_cast<const char *>(&head), sizeof(Head));
        return fwrite(fout, std::forward<Tail>(tail)...);
}

int main(int argc, char **argv) {
        constexpr
        std::array<GLfloat, 4> black{0, 0, 0, 1}, white{1, 1, 1, 1};
        try {
                fs::path output_dir{"osrtest"};
                fs::create_directories(output_dir);
                Eigen::MatrixXf vertices(2, 108);
                int ppm = int(std::round(360 * 1000 / 23.4));
                auto circle_point = [](const int t) {
                        return Eigen::Vector2f(10 * std::cos(float(M_PI / 180 * t)), 10 * std::sinf(float(M_PI / 180 * t)));
                };
                for (int i = 0; i < 36; i += 1) {
                        vertices.col(i * 3) = Eigen::Vector3f(0, 0);
                        vertices.col(i * 3 + 1) = circle_point(10 * i);
                        vertices.col(i * 3 + 2) = circle_point(10 * (i + 1));
                        
                }
                auto bmin = vertices.rowwise().minCoeff();
                auto bmax = vertices.rowwise().maxCoeff();
                auto sizes = bmax - bmin;
                vertices.colwise() -= 0.5f * (bmin + bmax);
                
                if (!::glfwInit()) {
                        throw std::runtime_error("glfwInit() failed");
                }
                ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // disable retina
                ::glfwWindowHint(GLFW_VISIBLE, 0);
                
                for (int k = 100; k < 3000; ++k) {
                        const Eigen::Vector2i size{k, k};
                        ::GLFWwindow *window = ::glfwCreateWindow(size.x(), size.y(), "offscreen", nullptr, nullptr);
                        if (!window) {
                                throw std::runtime_error("glfwCreateWindow() failed");
                        }
                        ::glfwMakeContextCurrent(window);
                        ::glClearColor(0, 0, 1, 1);
                        ::glEnable(GL_DEPTH_TEST);
                        ::glNewList(1, GL_COMPILE);
                        ::glBegin(GL_TRIANGLES);
                        for (int i = 0; i < vertices.cols(); i += 3) {
                                ::glColor3f(1, 1, 1);
                                ::glVertex2fv(vertices.col(i).data());
                                ::glVertex2fv(vertices.col(i + 1).data());
                                ::glVertex2fv(vertices.col(i + 2).data());
                        }
                        ::glEnd();
                        ::glEndList();
                        std::vector <uint8_t> line(static_cast<uint32_t> ((((size.x() + 7) / 8 + 3) / 4) * 4), 0x00);
                        std::vector <uint8_t> buffer(static_cast<uint32_t>(4 * size.x() * size.y()), 0x00); //color buffer
                        
                        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        ::glMatrixMode(GL_PROJECTION);
                        ::glLoadIdentity();
                        ::glOrtho(bmin.x(), bmax.x(), bmin.y(), bmax.y(), -1, 1);
                        ::glMatrixMode(GL_MODELVIEW);
                        ::glLoadIdentity();
                        ::glCallList(1);
                        ::glFlush();
                        ::glReadPixels(0, 0, size.x(), size.y(), GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
                        std::stringstream ss;
                        ss << output_dir.string() << "/image" << std::setw(5) << std::setfill('0') << k << ".bmp";
                        std::ofstream fout(ss.str(), std::ios::binary); // write to bmp
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
                        std::cerr << "\r" << std::setw(5) << k + 1 << "/" << 1000;
                        ::glfwDestroyWindow(window);
                        
                }
                ::glfwTerminate();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error." << std::endl;
        }
        return EXIT_SUCCESS;
}