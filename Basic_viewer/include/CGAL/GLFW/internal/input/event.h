#ifndef CGAL_GLFW_INTERNAL_EVENT_H
#define CGAL_GLFW_INTERNAL_EVENT_H

#include <variant>

#include "input_codes.h"

namespace CGAL {
namespace GLFW {

// forward declaration
class Basic_viewer; 

namespace internal {

struct Key_event { 
  Key_code key; 
  int scancode; 
  Action action; 
  Modifier mods; 
};

struct Mouse_btn_event { 
  Mouse_button button; 
  Action action; 
  Modifier mods; 
  double xpos, ypos; // mouse position 
  bool double_click; 
};

struct Scroll_event { 
  double xoffset; 
  double yoffset; 
  Modifier mods; 
};

struct Cursor_event { 
  double xpos;
  double ypos; 
};

struct Drag_event { 
  Mouse_button button; 
  Modifier mods; 
  double dx, dy; // mouse delta
  double xpos, ypos; // mouse position 
};

struct Resize_event { 
  int width; 
  int height; 
};

using Event = std::variant<Key_event, Mouse_btn_event, Scroll_event,  Cursor_event,  Drag_event, Resize_event>;

struct Event_context {
  Basic_viewer& viewer; 
  const Event& event;
  float dt = 0.f;
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_EVENT_H
