#ifndef CGAL_GLFW_INTERNAL_BINDING_IMPL_H
#define CGAL_GLFW_INTERNAL_BINDING_IMPL_H

#include <CGAL/basic.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "binding.h"
#include "Input.h"

#else 
#define CGAL_INLINE_FUNCTION
#endif

namespace CGAL {
namespace GLFW {
namespace internal {

namespace {

CGAL_INLINE_FUNCTION
std::string to_string(Modifier modifier) {
  std::string str; 
  std::vector<Modifier> list = internal::Input::modifier_list(modifier);
  for(const auto& modifier : list) {
    str += internal::Input::to_string_view(modifier); 
    str += " + ";  
  }

  return str; 
}

} // namespace

CGAL_INLINE_FUNCTION
bool Key_binding::operator==(const Key_binding& other) const { 
  return key == other.key && action == other.action && mods == other.mods; 
}

CGAL_INLINE_FUNCTION
bool Mouse_btn_binding::operator==(const Mouse_btn_binding& other) const { 
  return button == other.button && action == other.action && mods == other.mods && double_click == other.double_click; 
} 

CGAL_INLINE_FUNCTION
bool Scroll_binding::operator==(const Scroll_binding& other) const { 
  return mods == other.mods; 
}

CGAL_INLINE_FUNCTION
std::string to_string(const Key_binding& binding) {
  std::string str; 
  str += to_string(binding.mods);
  str += internal::Input::to_string_view(static_cast<Key_code>(binding.key)); 
  str += " ("; 
  str += Input::to_string_view(binding.action);
  str += ")"; 
  return str;
} 

std::ostream& operator<<(std::ostream& out, const Key_binding& binding) {
  return out << to_string(binding);
}

CGAL_INLINE_FUNCTION
std::string to_string(const Mouse_btn_binding& binding) {
  std::string str; 
  str += to_string(binding.mods);
  str += internal::Input::to_string_view(static_cast<Mouse_button>(binding.button)); 
  if (binding.double_click) {
    str += "(DOUBLE CLICK) ";
  } else {
    str += " ("; 
    str += Input::to_string_view(binding.action);
    str += ")";
  }   
  return str; 
} 

CGAL_INLINE_FUNCTION
std::ostream& operator<<(std::ostream& out, const Mouse_btn_binding& binding) {
  return out << to_string(binding);
}

CGAL_INLINE_FUNCTION
std::string to_string(const Scroll_binding& binding) {
  std::string str; 
  str += to_string(binding.mods);
  str += "SCROLL"; 
  return str;
}

CGAL_INLINE_FUNCTION
std::ostream& operator<<(std::ostream& out, const Scroll_binding& binding) {
  return out << to_string(binding);
}

CGAL_INLINE_FUNCTION
std::string to_string(const Binding& binding) {
  return std::visit([](const auto& b) { return to_string(b); }, binding);
}

CGAL_INLINE_FUNCTION
std::ostream& operator<<(std::ostream& out, const Binding& binding) {
  std::visit([&out](const auto& t) { out << t; }, binding);
  return out;
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_BINDING_IMPL_H
