#ifndef CGAL_GLFW_INTERNAL_WINDOW_H
#define CGAL_GLFW_INTERNAL_WINDOW_H

#include <CGAL/GLFW/bv_settings.h>

#include <GLFW/glfw3.h>

#include <optional>
#include <string>

#include "event.h"
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

class Window {
public: 
	using Key_event_fun = std::function<void(const internal::Key_event&)>; 
	using Mouse_btn_event_fun = std::function<void(const internal::Mouse_btn_event&)>; 
	using Scroll_event_fun = std::function<void(const internal::Scroll_event&)>; 
	using Resize_event_fun = std::function<void(const internal::Resize_event&)>; 
	using Cursor_event_fun = std::function<void(const internal::Cursor_event&)>; 

public:
	explicit Window(const Window_specification& spec={}); 
	
	Window(const Window&)=delete; 	
	Window& operator=(const Window&)=delete; 	
	Window(Window&&)=delete; 	
	Window& operator=(Window&&)=delete; 	
	
	~Window(); 

	void update() const; 

	GLFWwindow* handle() const;

  void on_key(Key_event_fun key_callback);
  void on_mouse_btn(Mouse_btn_event_fun mouse_btn_callback);
  void on_scroll(Scroll_event_fun scroll_callback);
  void on_resize(Resize_event_fun framebuffer_size_callback);
  void on_cursor_move(Cursor_event_fun cursor_callback);

	bool should_close() const; 
	void should_close(int value); 
	
	void window_pos(int& xpos, int& ypos) const; 
	void window_size(int& width, int& height) const; 
	
	float aspect_ratio() const;

	void framebuffer_size(int& width, int& height) const; 
	vec2i framebuffer_size() const; 
	int framebuffer_width() const; 
	int framebuffer_height() const; 

	void toggle_fullscreen();

private:
	void destroy(); 

	GLFWmonitor* current_monitor() const; 

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouse_btn_callback(GLFWwindow* window, int button, int action,int mods);
  static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
  static void cursor_callback(GLFWwindow* window, double xpos, double ypos);
	
  static void error_callback(int error, const char *description);

private: 
	GLFWwindow* handle_{ nullptr };
	std::string title_{}; 

	struct Window_rect { int x, y, width, height; }; 
	std::optional<Window_rect> window_rect_{ std::nullopt }; 

	vec2i framebuffer_size_{}; 
	float aspect_ratio_{ 1.0f }; 

  struct Click_tracker {
    double last_press_time{-1.0};
    int last_button{-1};
    static constexpr double DOUBLE_CLICK_THRESHOLD = 0.3; // 300ms
  };

  Click_tracker click_tracker_;

	Key_event_fun key_event_fun_{}; 
	Mouse_btn_event_fun mouse_btn_event_fun_{}; 
	Scroll_event_fun scroll_event_fun_{}; 
	Resize_event_fun resize_event_fun_{}; 
	Cursor_event_fun cursor_event_fun_{}; 
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Window_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_WINDOW_H
