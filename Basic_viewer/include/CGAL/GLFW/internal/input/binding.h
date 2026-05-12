#ifndef CGAL_GLFW_INTERNAL_BINDING_H
#define CGAL_GLFW_INTERNAL_BINDING_H

#include <ostream>
#include <variant>
#include <string>

#include <boost/container_hash/hash.hpp>

#include "input_codes.h"

namespace CGAL {
namespace GLFW {
namespace internal {

// To use lambda function with std::visit
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; }; 

// Some compilers might require this explicit deduction guide
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>; 

struct Key_binding { 
  Key_code key;
  Action action; 
  Modifier mods = Modifier::NONE; 

  bool operator==(const Key_binding& other) const; 
};

struct Mouse_btn_binding { 
  Mouse_button button; 
  Action action; 
  Modifier mods = Modifier::NONE; 
  bool double_click = false; 

  bool operator==(const Mouse_btn_binding& other) const;
};

struct Scroll_binding { 
  Modifier mods = Modifier::NONE; 

  bool operator==(const Scroll_binding& other) const;
};

using Binding = std::variant<Key_binding, Mouse_btn_binding, Scroll_binding>; 

std::string to_string(const Key_binding& binding);
std::ostream& operator<<(std::ostream& out, const Key_binding& binding); 

std::string to_string(const Mouse_btn_binding& binding); 
std::ostream& operator<<(std::ostream& out, const Mouse_btn_binding& binding); 

std::string to_string(const Scroll_binding& binding); 
std::ostream& operator<<(std::ostream& out, const Scroll_binding& binding); 

std::string to_string(const Binding& binding); 
std::ostream& operator<<(std::ostream& out, const Binding& binding); 

} // namespace internal
} // namespace GLFW 
} // namespace CGAL

// overriding std::hash for each Binding variant sub-type
namespace std {

template<> struct hash<CGAL::GLFW::internal::Key_binding> {
  size_t operator()(const CGAL::GLFW::internal::Key_binding& binding) const {
    size_t h = 0;
    boost::hash_combine(h, static_cast<int>(binding.key)); 
    boost::hash_combine(h, static_cast<int>(binding.action)); 
    boost::hash_combine(h, static_cast<int>(binding.mods)); 
    return h;  
  }
};

template<> struct hash<CGAL::GLFW::internal::Mouse_btn_binding> {
  size_t operator()(const CGAL::GLFW::internal::Mouse_btn_binding& binding) const {
    size_t h = 0;
    boost::hash_combine(h, static_cast<int>(binding.button)); 
    boost::hash_combine(h, static_cast<int>(binding.action)); 
    boost::hash_combine(h, static_cast<int>(binding.mods)); 
    boost::hash_combine(h, static_cast<int>(binding.double_click)); 
    return h;  
  }
};

template<> struct hash<CGAL::GLFW::internal::Scroll_binding> {
  size_t operator()(const CGAL::GLFW::internal::Scroll_binding& binding) const {
    return hash<int>{}(static_cast<int>(binding.mods)); 
  }
};

template<> struct hash<CGAL::GLFW::internal::Binding> {
  size_t operator()(const CGAL::GLFW::internal::Binding& binding) const {
    size_t h = std::visit(
      [](const auto& alt) {
        using T = std::decay_t<decltype(alt)>; 
        return hash<T>{}(alt); 
      }, 
    binding);
    boost::hash_combine(h, binding.index());
    return h; 
  }
};
} // namespace std

#ifdef CGAL_HEADER_ONLY
#include "binding_impl.h"
#endif // CGAL_HEADER_ONLY

#endif
