#ifndef CGAL_GLFW_INTERNAL_INPUT_H
#define CGAL_GLFW_INTERNAL_INPUT_H

#include <utility>
#include <vector>
#include <string_view>

#include <GLFW/glfw3.h>

#include "input_codes.h"

namespace CGAL {
namespace GLFW {
namespace internal {

struct Input {
  static bool is_key_pressed(GLFWwindow* window, Key_code key);
  static bool is_key_released(GLFWwindow* window, Key_code key);
  
  static bool is_mouse_button_pressed(GLFWwindow* window, Mouse_button button);
  static bool is_mouse_button_released(GLFWwindow* window, Mouse_button button);
  
  static std::pair<float, float> mouse_position(GLFWwindow* window);
  static float mouse_x(GLFWwindow* window);
  static float mouse_y(GLFWwindow* window);

  static Modifier active_modifiers(GLFWwindow* window); 

  static std::vector<Modifier> modifier_list(int mods); 

  static constexpr bool has_flag(int mods, int flag) {
    return (mods & flag) == flag;  
  } 

  static constexpr std::string_view to_string_view(Key_code key) {
    switch (key) {
      case Key_code::APOSTROPHE:       return "APOSTROPHE";
      case Key_code::COMMA:            return "COMMA";  
      case Key_code::MINUS:            return "MINUS";  
      case Key_code::PERIOD:           return "PERIOD";  
      case Key_code::SLASH:            return "SLASH"; 
      case Key_code::KEY_0:            return "KEY_0";
      case Key_code::KEY_1:            return "KEY_1";
      case Key_code::KEY_2:            return "KEY_2";
      case Key_code::KEY_3:            return "KEY_3";
      case Key_code::KEY_4:            return "KEY_4";
      case Key_code::KEY_5:            return "KEY_5";
      case Key_code::KEY_6:            return "KEY_6";
      case Key_code::KEY_7:            return "KEY_7";
      case Key_code::KEY_8:            return "KEY_8";
      case Key_code::KEY_9:            return "KEY_9";
      case Key_code::SEMICOLON:        return "SEMICOLON";  
      case Key_code::EQUAL:            return "EQUAL"; 
      case Key_code::A:                return "A";
      case Key_code::B:                return "B";
      case Key_code::C:                return "C";
      case Key_code::D:                return "D";
      case Key_code::E:                return "E";
      case Key_code::F:                return "F";
      case Key_code::G:                return "G";
      case Key_code::H:                return "H";
      case Key_code::I:                return "I";
      case Key_code::J:                return "J";
      case Key_code::K:                return "K";
      case Key_code::L:                return "L";
      case Key_code::M:                return "M";
      case Key_code::N:                return "N";
      case Key_code::O:                return "O";
      case Key_code::P:                return "P";
      case Key_code::Q:                return "Q";
      case Key_code::R:                return "R";
      case Key_code::S:                return "S";
      case Key_code::T:                return "T";
      case Key_code::U:                return "U";
      case Key_code::V:                return "V";
      case Key_code::W:                return "W";
      case Key_code::X:                return "X";
      case Key_code::Y:                return "Y";
      case Key_code::Z:                return "Z";
      case Key_code::LEFT_BRACKET:     return "LEFT_BRACK";  
      case Key_code::BACKSLASH:        return "BACKSLASH ";  
      case Key_code::RIGHT_BRACKET:    return "RIGHT_BRAC";  
      case Key_code::GRAVE_ACCENT:     return "GRAVE_ACCE";  
      case Key_code::SPACE:            return "SPACE";
      case Key_code::ESCAPE:           return "ESCAPE";
      case Key_code::ENTER:            return "ENTER";
      case Key_code::TAB:              return "TAB";
      case Key_code::BACKSPACE:        return "BACKSPACE";
      case Key_code::INSERT:           return "INSERT";
      case Key_code::DELETE:           return "DELETE";
      case Key_code::RIGHT:            return "RIGHT";
      case Key_code::LEFT:             return "LEFT";
      case Key_code::DOWN:             return "DOWN";
      case Key_code::UP:               return "UP";
      case Key_code::PAGE_UP:          return "PAGE_UP";
      case Key_code::PAGE_DOWN:        return "PAGE_DOWN";
      case Key_code::HOME:             return "HOME";
      case Key_code::END:              return "END";
      case Key_code::CAPS_LOCK:        return "CAPS_LOCK";
      case Key_code::SCROLL_LOCK:      return "SCROLL_LOCK";
      case Key_code::NUM_LOCK:         return "NUM_LOCK";
      case Key_code::PRINT_SCREEN:     return "PRINT_SCREEN";
      case Key_code::PAUSE:            return "PAUSE";
      case Key_code::F1:               return "F1";
      case Key_code::F2:               return "F2";
      case Key_code::F3:               return "F3";
      case Key_code::F4:               return "F4";
      case Key_code::F5:               return "F5";
      case Key_code::F6:               return "F6";
      case Key_code::F7:               return "F7";
      case Key_code::F8:               return "F8";
      case Key_code::F9:               return "F9";
      case Key_code::F10:              return "F10";
      case Key_code::F11:              return "F11";
      case Key_code::F12:              return "F12";
      case Key_code::F13:              return "F13";
      case Key_code::F14:              return "F14";
      case Key_code::F15:              return "F15";
      case Key_code::F16:              return "F16";
      case Key_code::F17:              return "F17";
      case Key_code::F18:              return "F18";
      case Key_code::F19:              return "F19";
      case Key_code::F20:              return "F20";
      case Key_code::F21:              return "F21";
      case Key_code::F22:              return "F22";
      case Key_code::F23:              return "F23";
      case Key_code::F24:              return "F24";
      case Key_code::F25:              return "F25";
      case Key_code::KP_0:             return "NUMP_0";
      case Key_code::KP_1:             return "NUMP_1";
      case Key_code::KP_2:             return "NUMP_2";
      case Key_code::KP_3:             return "NUMP_3";
      case Key_code::KP_4:             return "NUMP_4";
      case Key_code::KP_5:             return "NUMP_5";
      case Key_code::KP_6:             return "NUMP_6";
      case Key_code::KP_7:             return "NUMP_7";
      case Key_code::KP_8:             return "NUMP_8";
      case Key_code::KP_9:             return "NUMP_9";
      case Key_code::KP_DECIMAL:       return "NUMP_DECIMAL";
      case Key_code::KP_DIVIDE:        return "NUMP_DIVIDE";
      case Key_code::KP_MULTIPLY:      return "NUMP_MULTIPLY";
      case Key_code::KP_SUBTRACT:      return "NUMP_SUBTRACT";
      case Key_code::KP_ADD:           return "NUMP_ADD";
      case Key_code::KP_ENTER:         return "NUMP_ENTER";
      case Key_code::KP_EQUAL:         return "NUMP_EQUAL";
      case Key_code::LEFT_SHIFT:       return "LSHIFT";
      case Key_code::LEFT_CONTROL:     return "LCTRL";
      case Key_code::LEFT_ALT:         return "LALT";
      case Key_code::LEFT_SUPER:       return "LSUPER";
      case Key_code::RIGHT_SHIFT:      return "RSHIFT";
      case Key_code::RIGHT_CONTROL:    return "RCTRL";
      case Key_code::RIGHT_ALT:        return "RALT";
      case Key_code::RIGHT_SUPER:      return "RSUPER";
      case Key_code::MENU:             return "MENU";
      default:                         return "UNKNOWN";
    }
  }

  static constexpr std::string_view to_string_view(Mouse_button button) {
    switch(button) {
      case Mouse_button::LAST:   return "MOUSE_BTN_LAST"; 
      case Mouse_button::LEFT:   return "MOUSE_BTN_LEFT";
      case Mouse_button::RIGHT:  return "MOUSE_BTN_RIGHT";
      case Mouse_button::MIDDLE: return "MOUSE_BTN_MIDDLE";
      default:                   return "UNKNOWN";
    }
  }

  static constexpr std::string_view to_string_view(Modifier modifier) {
    switch(modifier) {
      case Modifier::SHIFT:     return "SHIFT";
      case Modifier::CONTROL:   return "CONTROL";
      case Modifier::ALT:       return "ALT";
      case Modifier::SUPER:     return "SUPER";
      default:                  return "NONE";
    }
  }

  static constexpr std::string_view to_string_view(Action action) {
    switch(action) {
      case Action::PRESS:     return "PRESS";
      case Action::RELEASE:   return "RELEASE";
      case Action::HOLD:      return "HOLD";
      default:                return "UNKNOWN";
    }
  }
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Input_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_INPUT_H
