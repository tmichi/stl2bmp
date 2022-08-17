/**
 * @file OffScreenRenderer.hpp
 * @brief Class declaration of mi4::OffScreenRenderer
 * @author Takashi Michikawa <michikawa@acm.org>
 * @copyright (c) 2016 -  Takashi Michikawa
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */

#ifndef MI_OFFSCREEN_RENDERER_HPP
#define MI_OFFSCREEN_RENDERER_HPP 1
#ifdef MI_GL_ENABLED // define MI_GL_ENABLED if you use this

#include <vector>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace mi {
        class FrameBufferObject {
        private:
                const int width_;
                const int height_;
                GLuint fbo_; // frame buffer object
                GLuint cbo_; //   +-- color buffer objcet
                GLuint rbo_; //   +-- render buffer object
        public:
                FrameBufferObject(const FrameBufferObject &that) = delete;
                
                FrameBufferObject(FrameBufferObject &&that) = delete;
                
                void operator=(const FrameBufferObject &that) = delete;
                
                void operator=(FrameBufferObject &&that) = delete;
                
                explicit FrameBufferObject(const int width = 128, const int height = 128) : width_(width), height_(height), fbo_(0), cbo_(0), rbo_(0) {
                        ::glGenTextures(1, &(this->cbo_));
                        ::glBindTexture(GL_TEXTURE_2D, this->cbo_);
                        ::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
                        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        ::glBindTexture(GL_TEXTURE_2D, 0);
                        ::glGenRenderbuffers(1, &(this->rbo_));
                        ::glBindRenderbuffer(GL_RENDERBUFFER, this->rbo_);
                        ::glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
                        ::glBindRenderbuffer(GL_RENDERBUFFER, 0);
                        ::glGenFramebuffers(1, &(this->fbo_));
                        ::glBindFramebuffer(GL_FRAMEBUFFER, this->fbo_);
                        ::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->cbo_, 0);
                        ::glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rbo_);
                        ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
                };
                
                ~FrameBufferObject() {
                        ::glDeleteFramebuffers(1, &(this->fbo_));
                        ::glDeleteRenderbuffers(1, &(this->rbo_));
                        ::glDeleteTextures(1, &(this->cbo_));
                }
                
                void activate() const {
                        ::glBindFramebuffer(GL_FRAMEBUFFER, this->fbo_);
                }
                
                void deactivate() const {
                        ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
                }
                
                inline int getWidth() const {
                        return this->width_;
                }
                
                inline int getHeight() const {
                        return this->height_;
                }
                
                void getBuffer(std::vector<GLfloat> &buffer) const {
                        buffer.assign(size_t(this->getWidth() * this->getHeight()) * size_t(4), 0);
                        ::glBindTexture(GL_TEXTURE_2D, this->cbo_);
                        ::glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &(buffer[0]));
                        ::glBindTexture(GL_TEXTURE_2D, 0);
                }
        };
        
        class OffScreenRenderer {
        private:
                const int width_;
                const int height_;
                GLFWwindow *window_;
        public:
                OffScreenRenderer &operator=(const OffScreenRenderer &) = delete;
                
                OffScreenRenderer &operator=(OffScreenRenderer &&) = delete;
                
                OffScreenRenderer(const OffScreenRenderer &) = delete;
                
                OffScreenRenderer(OffScreenRenderer &&) = delete;
                
                explicit OffScreenRenderer(const int width = 128, const int height = 128) : width_(width), height_(height), window_(nullptr) {
                        if (!::glfwInit()) {
                                throw std::runtime_error("glfwInit() failed");
                        }
                        ::glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
                        ::glfwWindowHint(GLFW_VISIBLE, 0);
                        this->window_ = ::glfwCreateWindow(width, height, "offscreen", nullptr, nullptr);
                        if (!this->window_) {
                                throw std::runtime_error("glfwCreateWindow() failed");
                        }
                        this->makeCurrent();
                }
                
                ~OffScreenRenderer() {
                        ::glfwTerminate();
                }
                
                [[nodiscard]] int width() const {
                        return this->width_;
                }
                
                [[nodiscard]] int height() const {
                        return this->height_;
                }
                
                GLFWwindow *window() {
                        return this->window_;
                }
                
                void swapBuffers() {
                        ::glfwSwapBuffers(this->window());
                }
                
                void makeCurrent() {
                        ::glfwMakeContextCurrent(this->window());
                }
        };
}
#endif//MI_GL_ENABLED
#endif
