#ifndef CGAL_GLFW_INTERNAL_INPUT_IMPL_H
#define CGAL_GLFW_INTERNAL_INPUT_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Input.h"

#else
#define CGAL_INLINE_FUNCTION
#endif

namespace CGAL {
namespace GLFW {
namespace internal {
  
CGAL_INLINE_FUNCTION
bool Input::is_key_pressed(GLFWwindow* window, Key_code key) { 
  return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
}

CGAL_INLINE_FUNCTION
bool Input::is_key_released(GLFWwindow* window, Key_code key) {
  return glfwGetKey(window, static_cast<int>(key)) == GLFW_RELEASE;
}
 
CGAL_INLINE_FUNCTION
bool Input::is_mouse_button_pressed(GLFWwindow* window, Mouse_button button) { 
  return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
}

CGAL_INLINE_FUNCTION
bool Input::is_mouse_button_released(GLFWwindow* window, Mouse_button button) {
  return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_RELEASE;
}
  
CGAL_INLINE_FUNCTION
std::pair<float, float> Input::mouse_position(GLFWwindow* window) {
  double xpos, ypos;  
  glfwGetCursorPos(window, &xpos, &ypos); 
  return { xpos, ypos };
}

CGAL_INLINE_FUNCTION
float Input::mouse_x(GLFWwindow* window) {
  return mouse_position(window).first;
}

CGAL_INLINE_FUNCTION
float Input::mouse_y(GLFWwindow* window) {
  return mouse_position(window).second;
}

CGAL_INLINE_FUNCTION
Modifier Input::active_modifiers(GLFWwindow* window) {
  Modifier mods = Modifier::NONE;
  if (Input::is_key_pressed(window, Key_code::LEFT_SHIFT) ||Input::is_key_pressed(window, Key_code::RIGHT_SHIFT))
    mods |= Modifier::SHIFT;
  if (Input::is_key_pressed(window, Key_code::LEFT_CONTROL) ||Input::is_key_pressed(window, Key_code::RIGHT_CONTROL))
    mods |= Modifier::CONTROL;
  if (Input::is_key_pressed(window, Key_code::LEFT_ALT) ||Input::is_key_pressed(window, Key_code::RIGHT_ALT))
    mods |= Modifier::ALT;
  if (Input::is_key_pressed(window, Key_code::LEFT_SUPER) ||Input::is_key_pressed(window, Key_code::RIGHT_SUPER))
    mods |= Modifier::SUPER;

  return mods; 
}

CGAL_INLINE_FUNCTION
std::vector<Modifier> Input::modifier_list(int mods) {
  std::vector<Modifier> list;
  list.reserve(6); 
  
  if (has_flag(mods, Modifier::SHIFT))
    list.emplace_back(Modifier::SHIFT); 
  if (has_flag(mods, Modifier::CONTROL))
  list.emplace_back(Modifier::CONTROL);
  if (has_flag(mods, Modifier::ALT))
  list.emplace_back(Modifier::ALT);
  if (has_flag(mods, Modifier::SUPER))
    list.emplace_back(Modifier::SUPER);

  return list; 
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_INPUT_IMPL_H
