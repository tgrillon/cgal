#ifndef CGAL_GLFW_INTERNAL_CONTEXT_WINDOW_IMPL_H
#define CGAL_GLFW_INTERNAL_CONTEXT_WINDOW_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Context_window.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <GLFW/glfw3.h>
#include <optional>
#include <glad/gl.h>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Context_window::Context_window(const Window_specification &spec)
    : title_(spec.title) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, spec.gl_major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, spec.gl_minor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, spec.gl_profile);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, spec.forward_compat);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, spec.debug_context);

  glfwWindowHint(GLFW_SAMPLES, spec.samples);
  glfwWindowHint(GLFW_VISIBLE, !spec.hidden);
  glfwWindowHint(GLFW_RESIZABLE, spec.resizable);

  handle_ = glfwCreateWindow(spec.width, spec.height, title_.c_str(), nullptr, nullptr);

  if (!handle_) {
    std::cerr << "Could not open GLFW window\n";
    return;
  }

  glfwMakeContextCurrent(handle_);
  if (!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "Failed to initialized GLAD!\n";
    destroy();
    return; 
  }

  glfwSwapInterval(spec.use_vsync);

  if (!spec.hidden) {
    // Print various OpenGL information to stdout
    std::cout << glGetString(GL_VENDOR) << ": " << glGetString(GL_RENDERER)
              << '\n';
    std::cout << "GLFW\t "    << glfwGetVersionString() << '\n';
    std::cout << "OpenGL\t "  << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL\t "    << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << "\n\n";
  }

  framebuffer_size(framebuffer_size_.x(), framebuffer_size_.y());
}

CGAL_INLINE_FUNCTION
Context_window::~Context_window() { destroy(); }

CGAL_INLINE_FUNCTION
void Context_window::update() const { glfwSwapBuffers(handle_); }

CGAL_INLINE_FUNCTION
void Context_window::destroy() {
  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
  }
}

CGAL_INLINE_FUNCTION
GLFWwindow *Context_window::handle() const { return handle_; }

CGAL_INLINE_FUNCTION
void Context_window::on_key(GLFWkeyfun key_callback) {
  glfwSetKeyCallback(handle_, key_callback);
}

CGAL_INLINE_FUNCTION
void Context_window::on_mouse_btn(GLFWmousebuttonfun mouse_btn_callback) {
  glfwSetMouseButtonCallback(handle_, mouse_btn_callback);
}

CGAL_INLINE_FUNCTION
void Context_window::on_cursor_move(GLFWcursorposfun cursor_callback) {
  glfwSetCursorPosCallback(handle_, cursor_callback);
}

CGAL_INLINE_FUNCTION
void Context_window::on_scroll(GLFWscrollfun scroll_callback) {
  glfwSetScrollCallback(handle_, scroll_callback);
}

CGAL_INLINE_FUNCTION
void Context_window::on_resize(GLFWframebuffersizefun framebuffer_size_callback) {
  glfwSetFramebufferSizeCallback(handle_, framebuffer_size_callback);
}

CGAL_INLINE_FUNCTION
bool Context_window::should_close() const {
  return glfwWindowShouldClose(handle_);
}

CGAL_INLINE_FUNCTION
void Context_window::should_close(int value) {
  glfwSetWindowShouldClose(handle_, value); 
}

CGAL_INLINE_FUNCTION
void Context_window::window_pos(int &xpos, int &ypos) const {
  glfwGetWindowPos(handle_, &xpos, &ypos);
}

CGAL_INLINE_FUNCTION
void Context_window::window_size(int &width, int &height) const {
  glfwGetWindowSize(handle_, &width, &height);
}

CGAL_INLINE_FUNCTION
void Context_window::framebuffer_size(int &width, int &height) const {
  glfwGetFramebufferSize(handle_, &width, &height);
}

CGAL_INLINE_FUNCTION
vec2i Context_window::framebuffer_size() const {
  return framebuffer_size_; 
}

CGAL_INLINE_FUNCTION
vec2i& Context_window::framebuffer_size() {
  return framebuffer_size_; 
}

CGAL_INLINE_FUNCTION
int Context_window::framebuffer_width() const {
  return framebuffer_size_.x();
} 

CGAL_INLINE_FUNCTION
int& Context_window::framebuffer_width() {
  return framebuffer_size_.x();
} 

CGAL_INLINE_FUNCTION
int Context_window::framebuffer_height() const {
  return framebuffer_size_.y();
} 

CGAL_INLINE_FUNCTION
int& Context_window::framebuffer_height() {
  return framebuffer_size_.y();
} 

CGAL_INLINE_FUNCTION
void Context_window::toggle_fullscreen() {
  GLFWmonitor *monitor = current_monitor();
  if (!monitor) return;
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  if (!mode) return;

  if (!window_rect_.has_value()) { // if in windowed mode
    auto& r = window_rect_.emplace();
    window_size(r.width, r.height);
    window_pos(r.x, r.y);
    glfwSetWindowMonitor(handle_, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
  } else {  // if in fullscreen mode
    glfwSetWindowMonitor(handle_, nullptr, window_rect_->x, window_rect_->y, window_rect_->width, window_rect_->height, GLFW_DONT_CARE);
    window_rect_.reset(); 
  }
}

CGAL_INLINE_FUNCTION
GLFWmonitor* Context_window::current_monitor() const {                                                                                                        
  int wx, wy, ww, wh;                                                                                                                                         
  window_pos(wx, wy);                                                                                                                        
  window_size(ww, wh);                                                                                                                       
                                                                                                                                                              
  int count = 0;                                                              
  GLFWmonitor** monitors = glfwGetMonitors(&count);                                                                                                           
  if (!monitors || count == 0) return glfwGetPrimaryMonitor();                                                                                                
  
  GLFWmonitor* best = nullptr;                                                                                                                                
  int best_overlap = -1;                                                      
  for (int i = 0; i < count; ++i) {                                                                                                                           
    int mx, my;                                                               
    glfwGetMonitorPos(monitors[i], &mx, &my);                                                                                                                 
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
    if (!mode) continue;                                                                                                                                      
                                                                              
    const int ox = std::max(0, std::min(wx + ww, mx + mode->width)  - std::max(wx, mx));                                                                      
    const int oy = std::max(0, std::min(wy + wh, my + mode->height) - std::max(wy, my));
    const int overlap = ox * oy;                                                                                                                              
    if (overlap > best_overlap) {                                             
      best_overlap = overlap;                                                                                                                                 
      best = monitors[i];                                                     
    }                                                                                                                                                         
  }                                                                           
  return best ? best : glfwGetPrimaryMonitor();
}


} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_CONTEXT_WINDOW_IMPL_H
