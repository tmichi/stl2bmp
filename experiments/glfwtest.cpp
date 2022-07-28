//
// Created by Takashi Michikawa on 2022/07/28.
//
#define _USE_MATH_DEFINES 1

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
#endif

#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <array>
#include <stdexcept>
#include <GLFW/glfw3.h>

int main() {
        try {
                if (!::glfwInit()) {
                        throw std::runtime_error("glfwInit() failed");
                }
                ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE); // disable retina
                int mins = 50;
                int maxs = 2000;
                ::GLFWwindow *window = ::glfwCreateWindow(mins, mins, "glfwtest", NULL, NULL);
                if (!window) {
                        throw std::runtime_error("glfwCreateWindow() failed");
                }
                ::glfwMakeContextCurrent(window);
                for (int k = mins; k < maxs; ++k) {
                        if (::glfwWindowShouldClose(window)) {
                                break;
                        }
                        std::stringstream ss;
                        ss << std::setw(5) << k << "x" << std::setw(5) << k;
                        ::glfwSetWindowTitle(window, ss.str().c_str());
                        ::glfwSetWindowSize(window, k, k);
                        ::glViewport(0, 0, k, k);
                        ::glClearColor(0, 0, 1, 1);
                        ::glEnable(GL_DEPTH_TEST);
                        ::glNewList(1, GL_COMPILE);
                        ::glBegin(GL_POLYGON);
                        ::glColor3f(1, 1, 1);
                        for (int i = 0; i < 360; i += 10) {
                                ::glVertex2d(10 * std::cos(M_PI / 180 * i), 10 * std::sin(float(M_PI / 180 * i)));
                        }
                        ::glEnd();
                        ::glEndList();
                        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        ::glMatrixMode(GL_PROJECTION);
                        ::glLoadIdentity();
                        ::glOrtho(-10, 10, -10, 10, -1, 1);
                        ::glMatrixMode(GL_MODELVIEW);
                        ::glLoadIdentity();
                        ::glCallList(1);
                        ::glfwSwapBuffers(window);
                        ::glfwPollEvents();
                }
                ::glfwTerminate();
        } catch (std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
        } catch (...) {
                std::cerr << "Unknown error." << std::endl;
        }
        return EXIT_SUCCESS;
}