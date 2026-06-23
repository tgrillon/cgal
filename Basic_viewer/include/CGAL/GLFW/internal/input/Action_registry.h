#ifndef CGAL_GLFW_INTERNAL_ACTION_REGISTRY_H
#define CGAL_GLFW_INTERNAL_ACTION_REGISTRY_H

#include <CGAL/config.h>

#include <unordered_map>
#include <functional>
#include <optional>
#include <variant>
#include <string>
#include <map>

#include "event.h"
#include "binding.h"

namespace CGAL {
namespace GLFW {
namespace internal {

using Event_callback = std::function<bool(Event_context&)>; // should return true if the action implies a redraw  

struct Action_entry {
  std::string action_name {}; 
  std::string section_name {}; 
  std::string description {}; 
  Binding binding {}; 
  Event_callback callback {};
};

struct Drag_handlers {
  std::function<void(Event_context&, float xpos, float ypos)> on_begin;
  std::function<bool(Event_context&, float xpos, float ypos, float dx, float dy)> on_drag;
  std::function<void(Event_context&)> on_end;
};

class Action_registry {
public: // Public methods
  void register_action(const std::string& section_name, const std::string& action_name, const Binding& binding, const std::string& description, Event_callback callback); 

  void register_drag_action(const std::string& section_name, const std::string& action_name, const Mouse_btn_binding& binding, const std::string& description, const Drag_handlers& handlers);

  bool dispatch(Event_context& context);
  
  std::optional<Binding> binding_of(const std::string& section_name, const std::string& action_name) const;
  bool rebind(const std::string& section_name, const std::string& action_name, const Binding& new_binding); 
  
  void print_help() const;
  
  template<class B, class F>
  void for_each_hold(F&& f) const; 

public: // Public static methods 
  static bool valid_binding(const Binding& binding); 

private: // Private methods
  std::optional<Binding> extract_binding(const Event& event) const; 

private: // Alisases
  using Section_name = std::string; 
  using Action_name = std::string; 

private: // Attributes
  std::unordered_map<Binding, Action_entry> binding_to_action_ {}; 
  std::map<Section_name, std::unordered_map<Action_name, Binding>> actions_by_section_ {}; 
};

template<class B, class F>
void Action_registry::for_each_hold(F&& f) const {
  for (const auto&[binding, entry] : binding_to_action_) {
    if (std::holds_alternative<B>(binding)) {
      const auto& b = std::get<B>(binding); 
      if (b.action == Action::HOLD && f(b)) break;   
    }
  }
} 

} // namespace internal
} // namespace GLFW
} // namespace CGAL
 
#ifdef CGAL_HEADER_ONLY
#include "Action_registry_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_ACTION_REGISTRY_H
