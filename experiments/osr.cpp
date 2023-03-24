//
// Created by Takashi Michikawa on 2022/07/28.
//
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define _USE_MATH_DEFINES 1
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
#endif

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <array>
#include <vector>
#include <stdexcept>

#define MI_GL_ENABLED

#include "../OffScreenRenderer.hpp"
#include <GLFW/glfw3.h>

namespace fs = std::filesystem;

namespace mi_stl2bmp {
        void fwrite([[maybe_unused]]std::ofstream &fout) {}
        
        template<class Head, class... Tail>
        void fwrite(std::ofstream &fout, Head &&head, Tail &&... tail) {
                fout.write(reinterpret_cast<const char *>(&head), sizeof(Head));
                fwrite(fout, std::forward<Tail>(tail)...);
        }
}
int main() {
        try {
                fs::path output_dir{"osrtest"};
                fs::create_directories(output_dir);
                const int ppm = int(std::round(360 * 1000 / 23.4));
                if (!::glfwInit()) {
                        throw std::runtime_error("glfwInit() failed");
                }
                ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // disable retina
                ::glfwWindowHint(GLFW_VISIBLE, 0);
                
                int mins = 50;
                int maxs = 3000;
                for (int k = mins; k < maxs; ++k) {
                        ::GLFWwindow *window = ::glfwCreateWindow(k, k, "offscreen", nullptr, nullptr);
                        if (!window) {
                                throw std::runtime_error("glfwCreateWindow() failed");
                        }
                        ::glfwMakeContextCurrent(window);
        
                        if (GLenum err = ::glewInit(); err != GLEW_OK) {
                                throw std::runtime_error(reinterpret_cast<const char *>(::glewGetErrorString(err)));
                        }
        
                        mi::FrameBufferObject fbo(k, k);
                        fbo.activate();
        
                        int w, h;
                        glfwGetFramebufferSize(window, &w, &h);
                        glViewport(0, 0, k, k);
                        std::cerr << w << " " << h << " " << k << std::endl;
                        ::glfwMakeContextCurrent(window);
                        ::glClearColor(0, 0, 1, 1);
                        ::glEnable(GL_DEPTH_TEST);
                        ::glNewList(1, GL_COMPILE);
                        ::glBegin(GL_POLYGON);
                        ::glColor3f(1, 1, 1);
                        for (int i = 0; i < 360; i += 1) {
                                ::glVertex2d(10 * std::cos(M_PI / 180 * i), 10 * std::sin(M_PI / 180 * i));
                        }
                        ::glEnd();
                        ::glEndList();
                        std::vector<uint8_t> line(static_cast<uint32_t> ((((k + 7) / 8 + 3) / 4) * 4), 0x00);
                        //std::vector<uint8_t> buffer(static_cast<uint32_t>(4 * k * k), 0x00); //color buffer
                        std::vector<float> buffer(static_cast<uint32_t>(4 * k * k), 0); //color buffer
        
                        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        ::glMatrixMode(GL_PROJECTION);
                        ::glLoadIdentity();
                        ::glOrtho(-10, 10, -10, 10, -1, 1);
                        ::glMatrixMode(GL_MODELVIEW);
                        ::glLoadIdentity();
                        ::glCallList(1);
                        ::glFlush();
                        fbo.getBuffer(buffer);
                        //::glReadPixels(0, 0, k, k, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);
                        std::stringstream ss;
                        ss << output_dir.string() << "/image" << std::setw(5) << std::setfill('0') << k << ".bmp";
                        std::ofstream fout(ss.str(), std::ios::binary); // write to bmp
                        if (!fout) {
                                throw std::runtime_error(ss.str() + " cannot be open");
                        } else {
                                const uint32_t headerSize = 14u + 40u + 2 * 4u;
                                const uint32_t imageSize = uint32_t(k) * uint32_t(line.size());
                                mi_stl2bmp::fwrite(fout, uint16_t(0x4D42), headerSize + imageSize, uint16_t(0), uint16_t(0), headerSize);
                                mi_stl2bmp::fwrite(fout, uint32_t(40), k, k, uint16_t(1), uint16_t(1), uint32_t(0), imageSize, ppm, ppm, uint32_t(2), uint32_t(0u));
                                mi_stl2bmp::fwrite(fout, uint32_t(0x00000000), uint32_t(0x00ffffff)); //Palette 1
                                for (uint16_t y = 0; y < uint16_t(k); y++) {
                                        std::fill(line.begin(), line.end(), 0x00);
                                        for (uint16_t x = 0; x < uint16_t(k); ++x) {
                                                if (buffer[uint32_t(4 * (x + k * y))] > 0) {
                                                        line[x / 8] |= 0x01 << (7 - x % 8);
                                                }
                                        }
                                        fout.write(reinterpret_cast<char *>(line.data()), std::streamsize(line.size()));
                                }
                        }
                        std::cerr << "\r" << std::setw(5) << k << "/" << maxs;
                        ::glfwDestroyWindow(window);
                }
                std::cerr << std::endl;
                ::glfwTerminate();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error." << std::endl;
        }
        return EXIT_SUCCESS;
}