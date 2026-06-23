#ifndef CGAL_GLFW_INTERNAL_ACTION_REGISTRY_IMPL_H
#define CGAL_GLFW_INTERNAL_ACTION_REGISTRY_IMPL_H

#include "binding.h"
#include "event.h"
#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Action_registry.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <string_view>
#include <iostream>
#include <iomanip>

#include <CGAL/assertions.h>

#include "Input.h"
#include "input_codes.h"

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
void Action_registry::register_action(
  const std::string& section_name, 
  const std::string& action_name, 
  const Binding& binding, 
  const std::string& description, 
  Event_callback callback) {
  // Check invariants

  // Check section name validity (empty string is invalid)
  CGAL_assertion_msg(!section_name.empty(), "can't register an action in a section with an empty name");
  if (section_name.empty()) return; 

  // Check action name validity (empty string is invalid)
  CGAL_assertion_msg(!action_name.empty(), "can't register an action with an empty name");
  if (action_name.empty()) return; 

  // Check binding validity (no Key_code::UNKNOWN or Mouse_button::UNKNOWN)
  const bool is_valid_binding = valid_binding(binding);  
  CGAL_assertion_msg(is_valid_binding, ("invalid binding: [" + to_string(binding) + "]").c_str());
  if (!is_valid_binding) return; 

  // Check that the binding is not already set to another registered action 
  auto action_it = binding_to_action_.find(binding); 
  const bool binding_already_assigned = (action_it != binding_to_action_.end());  
  CGAL_assertion_msg(!binding_already_assigned, ("binding already assigned to [" + action_it->second.section_name + "." + action_it->second.action_name + "]").c_str());
  if (binding_already_assigned) return;  

  // Check action name availability
  auto& actions = actions_by_section_[section_name];
  auto binding_it = actions.find(action_name);
  const bool action_exist = (binding_it != actions.end()); 
  CGAL_assertion_msg(!action_exist, ("[" + action_name + "] already registered with [" + to_string(binding_it->second) + "]").c_str());  
  if (action_exist) return; 

  // Register new entry
  
  Action_entry entry{
    .action_name = action_name, 
    .section_name = section_name, 
    .description = description, 
    .binding = binding, 
    .callback = std::move(callback) 
  };
  
  binding_to_action_.emplace(binding, std::move(entry)); 
  actions_by_section_[section_name].emplace(action_name, binding); 
} 

CGAL_INLINE_FUNCTION
void Action_registry::register_drag_action(
  const std::string& section_name, 
  const std::string& action_name, 
  const Mouse_btn_binding& binding, 
  const std::string& description, 
  const Drag_handlers& handlers) {
  
    register_action(
      section_name, 
      action_name + "_begin", 
      Mouse_btn_binding{ binding.button, Action::PRESS, binding.mods }, 
      "", 
      [on_begin = std::move(handlers.on_begin)](Event_context& ctx) {
        const auto& e = std::get<Mouse_btn_event>(ctx.event); 
        on_begin(ctx, e.xpos, e.ypos); 
        return false; 
      });

    register_action(
      section_name, 
      action_name, 
      Mouse_btn_binding{ binding.button, Action::HOLD, binding.mods }, 
      description, 
      [on_drag = std::move(handlers.on_drag)](Event_context& ctx) {
        const auto& e = std::get<Drag_event>(ctx.event); 
        return on_drag(ctx, e.xpos, e.ypos, e.dx, e.dy); 
      });

    
    register_action(
      section_name, 
      action_name + "_end", 
      Mouse_btn_binding{ binding.button, Action::RELEASE, binding.mods }, 
      "", 
      [on_end = std::move(handlers.on_end)](Event_context& ctx) {
        on_end(ctx);
        return false;  
      });
}

CGAL_INLINE_FUNCTION
bool Action_registry::dispatch(Event_context& context) {
  // Check if the event should be handled
  auto binding = extract_binding(context.event);
  if (!binding.has_value()) return false; 

  // Check if the binding is registered
  auto it = binding_to_action_.find(binding.value());
  if (it == binding_to_action_.end()) return false; 

  // Check if a callback function is attached to it
  const Action_entry& entry = it->second; 
  if (!entry.callback) return false;

  // Trigger the callback function
  return entry.callback(context);
}

CGAL_INLINE_FUNCTION
std::optional<Binding> Action_registry::binding_of(const std::string& section_name, const std::string& action_name) const {
  // Check if the section exist
  auto section_it = actions_by_section_.find(section_name);
  if (section_it == actions_by_section_.end()) return std::nullopt; 
  
  const auto& actions = section_it->second; 

  // Check if the action exist within this section
  auto action_it = actions.find(action_name);
  if (action_it == actions.end()) return std::nullopt; 

  // Return the attached binding
  return action_it->second;
}

CGAL_INLINE_FUNCTION
bool Action_registry::rebind(const std::string& section_name, const std::string& action_name, const Binding& new_binding) {
  // Check if the section exist
  auto section_it = actions_by_section_.find(section_name); 
  const bool section_exist = (section_it != actions_by_section_.end());
  CGAL_assertion_msg(section_exist, ("unknown section [" + section_name + "]").c_str());
  if (!section_exist) return false; 
  
  auto& actions = section_it->second; 

  // Check if the action exist within this section
  auto action_it = actions.find(action_name);
  const bool action_exist = (action_it != actions.end());  
  CGAL_assertion_msg(action_exist, ("unknown action [" + section_name + "." + action_name + "]").c_str());
  if (!action_exist) return false; 

  { // Check that we don't rebind with an existing binding   
    auto it = binding_to_action_.find(new_binding); 
    const bool binding_already_used = (it != binding_to_action_.end());
    CGAL_assertion_msg(!binding_already_used, ("[" + to_string(new_binding) + "] already binded to [" + it->second.action_name + "]").c_str());  
    if (binding_already_used) return false;
  }

  // Update node in binding_to_action 
  const auto& old_binding = action_it->second;
  auto node = binding_to_action_.extract(old_binding); 
  CGAL_assertion(!node.empty());
  node.key() = new_binding; 
  node.mapped().binding = new_binding; 
  binding_to_action_.insert(std::move(node)); 

  // Update name index
  action_it->second = new_binding;
  return true;
} 

CGAL_INLINE_FUNCTION
void Action_registry::print_help() const {
  std::cout << "Basic Viewer GLFW - Features shortcuts  :" << "\n\n";

  const int n = 140 - 2;
  const std::string section_footer(n, '=');

  for (const auto& [section_title, actions] : actions_by_section_) {
    std::string section_header = " Section : " + section_title + " ";

    const int nb_iterations = n - static_cast<int>(section_header.length());
    for (int i = 0; i < nb_iterations / 4; ++i) {
      section_header = "=" + section_header + "===";
    }
    std::cout << section_header << "\n";

    for (const auto& [action_name, binding] : actions) {
      auto entry_it = binding_to_action_.find(binding);
      CGAL_assertion(entry_it != binding_to_action_.end());
      const std::string& description = entry_it->second.description;

      if (description.empty()) {
        std::cout << '\n';
        continue;
      }

      std::cout << std::setw(n / 4 + 4) << std::right << to_string(binding)
                << " - " << action_name << " : " << description << '\n';
    }

    std::cout << section_footer << "\n";
  }
  std::cout << std::endl;
}

CGAL_INLINE_FUNCTION
bool Action_registry::valid_binding(const Binding& binding) {
  return std::visit(overloaded{
      [](const Key_binding& b)        { return static_cast<Key_code>(b.key) != Key_code::UNKNOWN; },  
      [](const Mouse_btn_binding& b)  {return static_cast<Mouse_button>(b.button) != Mouse_button::UNKNOWN;},  
      [](const Scroll_binding& b)     { return true; }, 
      [](const auto& b)     { return false; } 
  }, 
    binding
  ); 
}

CGAL_INLINE_FUNCTION
std::optional<Binding> Action_registry::extract_binding(const Event& event) const {
  return std::visit(overloaded{
      [](const Key_event& e) -> std::optional<Binding>        { return Key_binding{ .key = e.key, .action = e.action , .mods = e.mods}; },
      [](const Mouse_btn_event& e) -> std::optional<Binding>  { return Mouse_btn_binding{ .button = e.button, .action = e.action, .mods = e.mods, .double_click = e.double_click }; },
      [](const Scroll_event& e) -> std::optional<Binding>     { return Scroll_binding{ .mods = e.mods }; },
      [](const Drag_event& e) -> std::optional<Binding>       { return Mouse_btn_binding{ .button = e.button, .action = Action::HOLD, .mods = e.mods }; },
      [](const auto&) -> std::optional<Binding>               { return std::nullopt; }
  },
    event
  );
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL


#endif // CGAL_GLFW_INTERNAL_ACTION_REGISTRY_IMPL_H
