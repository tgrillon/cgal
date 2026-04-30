#ifndef CGAL_GLFW_INTERNAL_WINDOW_IMPL_H
#define CGAL_GLFW_INTERNAL_WINDOW_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include <stdexcept>
#include <iostream>

#include "Window.h"
#include "Input.h"
#include "event.h"

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
Window::Window(const Window_specification &spec)
  : title_(spec.title) {
  // Initialise GLFW
  if (!glfwInit()) {
    throw std::runtime_error("Could not start GLFW");  
  }

  glfwSetErrorCallback(error_callback);

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
    destroy(); 
    throw std::runtime_error("Could not open GLFW window");  
  }

  glfwSetWindowUserPointer(handle_, this); 

  glfwMakeContextCurrent(handle_);
  if (!gladLoadGL(glfwGetProcAddress)) {
    destroy();
    throw std::runtime_error("Failed to initialized GLAD"); 
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
Window::~Window() { destroy(); }

CGAL_INLINE_FUNCTION
void Window::update() const { glfwSwapBuffers(handle_); }

CGAL_INLINE_FUNCTION
void Window::destroy() {
  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
  }
  
  glfwTerminate();
}

CGAL_INLINE_FUNCTION
GLFWwindow *Window::handle() const { return handle_; }

CGAL_INLINE_FUNCTION
void Window::on_key(Window::Key_event_fun fun) {
  key_event_fun_ = std::move(fun);
  glfwSetKeyCallback(handle_, key_callback);
}

CGAL_INLINE_FUNCTION
void Window::on_mouse_btn(Window::Mouse_btn_event_fun fun) {
  mouse_btn_event_fun_ = std::move(fun);
  glfwSetMouseButtonCallback(handle_, mouse_btn_callback);
}

CGAL_INLINE_FUNCTION
void Window::on_cursor_move(Window::Cursor_event_fun fun) {
  cursor_event_fun_ = std::move(fun);
  glfwSetCursorPosCallback(handle_, cursor_callback);
}

CGAL_INLINE_FUNCTION
void Window::on_scroll(Window::Scroll_event_fun fun) {
  scroll_event_fun_ = std::move(fun);
  glfwSetScrollCallback(handle_, scroll_callback);
}

CGAL_INLINE_FUNCTION
void Window::on_resize(Window::Resize_event_fun fun) {
  resize_event_fun_ = std::move(fun);
  glfwSetFramebufferSizeCallback(handle_, framebuffer_size_callback);
}

CGAL_INLINE_FUNCTION
bool Window::should_close() const {
  return glfwWindowShouldClose(handle_);
}

CGAL_INLINE_FUNCTION
void Window::should_close(int value) {
  glfwSetWindowShouldClose(handle_, value); 
}

CGAL_INLINE_FUNCTION
void Window::window_pos(int &xpos, int &ypos) const {
  glfwGetWindowPos(handle_, &xpos, &ypos);
}

CGAL_INLINE_FUNCTION
float Window::aspect_ratio() const {
  return aspect_ratio_; 
}

CGAL_INLINE_FUNCTION
void Window::window_size(int &width, int &height) const {
  glfwGetWindowSize(handle_, &width, &height);
}

CGAL_INLINE_FUNCTION
void Window::framebuffer_size(int &width, int &height) const {
  glfwGetFramebufferSize(handle_, &width, &height);
}

CGAL_INLINE_FUNCTION
vec2i Window::framebuffer_size() const {
  return framebuffer_size_; 
}

CGAL_INLINE_FUNCTION
int Window::framebuffer_width() const {
  return framebuffer_size_.x();
} 

CGAL_INLINE_FUNCTION
int Window::framebuffer_height() const {
  return framebuffer_size_.y();
} 

CGAL_INLINE_FUNCTION
void Window::toggle_fullscreen() {
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
GLFWmonitor* Window::current_monitor() const {                                                                                                        
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

CGAL_INLINE_FUNCTION
void Window::error_callback(int error, const char *description) {
  std::cerr << "GLFW returned an error:\n\t" << description << "(" << error << ")\n";
}

CGAL_INLINE_FUNCTION
void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto self = static_cast<Window*>(glfwGetWindowUserPointer(window)); 
  
  // Re-map to keycap-based key code so bindings are layout-independent.
  if (const char* name = glfwGetKeyName(key, scancode)) {
    if (name && name[0] != '\0' && name[1] == '\0') {
      char c = name[0]; 
      if (c >= 'a' && c <= 'z') {
        c = static_cast<int>(c - 'a' + 'A'); 
      }
      // Only override for the printable ASCII range that overlaps with Key_code. 
      if (c >= 0x20 /*32*/ && c <= 0x60 /*96*/) {
        key = static_cast<int>(c); 
      }
    }
  }

  internal::Key_event event{
    .key = static_cast<internal::Key_code>(key), 
    .scancode = scancode, 
    .action = static_cast<internal::Action>(action), 
    .mods = static_cast<internal::Modifier>(mods)};

  self->key_event_fun_(event); 
}

CGAL_INLINE_FUNCTION
void Window::mouse_btn_callback(GLFWwindow* window, int button, int action,int mods) {
  auto self = static_cast<Window*>(glfwGetWindowUserPointer(window)); 
  
  internal::Mouse_btn_event event{
    .button = static_cast<internal::Mouse_button>(button),
    .action = static_cast<internal::Action>(action), 
    .mods = static_cast<internal::Modifier>(mods)
  };

  if (action == GLFW_PRESS) {
    // Handle double click event
    auto &tracker = self->click_tracker_;
    double current_time = glfwGetTime();
    bool double_click;
    if (button == tracker.last_button &&
        (current_time - tracker.last_press_time) <
            Click_tracker::DOUBLE_CLICK_THRESHOLD) {
      double_click = true;
    } else {
      double_click = false;
    }

    tracker.last_press_time = double_click ? -1.0 : current_time;
    tracker.last_button = button;

    event.double_click = double_click;
  }

  self->mouse_btn_event_fun_(event);
}

CGAL_INLINE_FUNCTION
void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  auto self = static_cast<Window*>(glfwGetWindowUserPointer(window)); 
  
  // Add modifier information to scroll event
  internal::Modifier mods = internal::Input::active_modifiers(self->handle_); 

  internal::Scroll_event event{
    .xoffset = xoffset, 
    .yoffset = yoffset, 
    .mods = static_cast<internal::Modifier>(mods)};

  self->scroll_event_fun_(event);
}

CGAL_INLINE_FUNCTION
void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  auto self = static_cast<Window*>(glfwGetWindowUserPointer(window)); 
  
  self->framebuffer_size_ = { width, height };
  self->aspect_ratio_ = static_cast<float>(self->framebuffer_size_.x()) / self->framebuffer_size_.y();
  self->resize_event_fun_({ width, height });
}

CGAL_INLINE_FUNCTION
void Window::cursor_callback(GLFWwindow* window, double xpos, double ypos) {
  auto self = static_cast<Window*>(glfwGetWindowUserPointer(window)); 
  self->cursor_event_fun_({ xpos, ypos }); 
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_WINDOW_IMPL_H
