#ifndef CGAL_GLFW_INTERNAL_CONTEXT_WINDOW_H
#define CGAL_GLFW_INTERNAL_CONTEXT_WINDOW_H

#include <CGAL/GLFW/bv_settings.h>

#include <GLFW/glfw3.h>

#include <optional>
#include <string>
#include <functional>

#include "utils.h"

namespace CGAL {
namespace GLFW {
namespace internal {

struct Window_specification {
	std::string title;
	int width = 1280; 
	int height = 720; 
	int gl_major = 3; 
	int gl_minor = 3; 
	int gl_profile = GLFW_OPENGL_CORE_PROFILE;
	int samples = CGAL_WINDOW_SAMPLES;
	bool debug_context = true; 
	bool forward_compat = true; 
	bool use_vsync = true; 
	bool hidden = false; 
	bool resizable = true; 
};

class Context_window {
public: 
	struct Size { int width, height; }; 

public:
	Context_window(const Window_specification& spec); 
	~Context_window(); 

	void update() const; 
	void destroy(); 

	GLFWwindow* handle() const;

  void on_key(GLFWkeyfun key_callback);
  void on_mouse_btn(GLFWmousebuttonfun mouse_btn_callback);
  void on_cursor_move(GLFWcursorposfun cursor_callback);
  void on_scroll(GLFWscrollfun scroll_callback);
  void on_resize(GLFWframebuffersizefun framebuffer_size_callback);

	bool should_close() const; 
	void should_close(int value); 
	
	void window_pos(int& xpos, int& ypos) const; 
	void window_size(int& width, int& height) const; 

	void framebuffer_size(int& width, int& height) const; 
	vec2i framebuffer_size() const; 
	vec2i& framebuffer_size(); 
	int framebuffer_width() const; 
	int& framebuffer_width(); 
	int framebuffer_height() const; 
	int& framebuffer_height(); 

	void toggle_fullscreen();

private:
	GLFWmonitor* current_monitor() const; 

private: 
	GLFWwindow* handle_{ nullptr };
	std::string title_{}; 

	struct Window_rect { int x, y, width, height; }; 
	std::optional<Window_rect> window_rect_{ std::nullopt }; 

	vec2i framebuffer_size_{}; 
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Context_window_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_CONTEXT_WINDOW_H
