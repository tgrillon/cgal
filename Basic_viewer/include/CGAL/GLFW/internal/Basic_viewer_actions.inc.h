#ifndef CGAL_GLFW_INTERNAL_BASIC_VIEWER_ACTIONS_SPLIT_H
#define CGAL_GLFW_INTERNAL_BASIC_VIEWER_ACTIONS_SPLIT_H

#include <CGAL/GLFW/internal/binding.h>
#include <CGAL/basic.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include <CGAL/GLFW/Basic_viewer.h>

#else
#define CGAL_INLINE_FUNCTION
#endif

namespace CGAL {
namespace GLFW {

CGAL_INLINE_FUNCTION
void Basic_viewer::register_clipping_plane_actions() const {

  const std::string& section_name = "Clipping_plane";

  // Drag bindings

  action_registry_->register_action(
    section_name,
    "rotate",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::LEFT, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Rotate the clipping plane",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Drag_event>(ctx.event);
      ctx.viewer.rotate_clipping_plane(e.dx, e.dy);
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "translate",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::RIGHT, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Translate the clipping plane",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Drag_event>(ctx.event);
      ctx.viewer.translate_clipping_plane(e.dx, e.dy);
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "translate_along_camera_direction",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::MIDDLE, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Translate the clipping plane along camera direction",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Drag_event>(ctx.event);
      ctx.viewer.translate_clipping_plane_along_camera(e.dx, e.dy);
      return true; // need to redraw
    });

  // Scroll binding

  action_registry_->register_action(
    section_name,
    "translate_along_normal",
    internal::Scroll_binding{ .mods = internal::Modifier::CONTROL },
    "Translate the clipping plane along its normal",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Scroll_event>(ctx.event);
      ctx.viewer.clipping_plane_->translation(
        static_cast<float>(e.yoffset) * CGAL_CLIPPING_PLANE_NORMAL_TRANSLATION_SPEED);
      return true; // need to redraw
    });

  // Key bindings

  action_registry_->register_action(
    section_name,
    "switch_display_mode",
    internal::Key_binding{ .key = internal::Key_code::C, .action = internal::Action::RELEASE },
    "Switch clipping plane display mode",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->switch_display_mode(); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "toggle_rendering",
    internal::Key_binding{ .key = internal::Key_code::C, .action = internal::Action::RELEASE, .mods = internal::Modifier::ALT },
    "Toggle clipping plane rendering on/off",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->toggle_rendering();
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "switch_constraint_axis",
    internal::Key_binding{ .key = internal::Key_code::A, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL },
    "Switch constraint axis for clipping plane rotation",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->switch_constraint_axis(); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "reset",
    internal::Key_binding{ .key = internal::Key_code::R, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL | internal::Modifier::ALT },
    "Reset clipping plane",
    [](internal::Event_context& ctx) { 
      ctx.viewer.reset_clipping_plane(); 
      return true; // need to redraw
    });

  // HOLD key bindings

  action_registry_->register_action(
    section_name,
    "increase_translation_speed",
    internal::Key_binding{ .key = internal::Key_code::X, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Increase translation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->increase_translation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "increase_rotation_speed",
    internal::Key_binding{ .key = internal::Key_code::R, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Increase rotation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->increase_rotation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "decrease_translation_speed",
    internal::Key_binding{ .key = internal::Key_code::X, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT | internal::Modifier::CONTROL },
    "Decrease translation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->decrease_translation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "decrease_rotation_speed",
    internal::Key_binding{ .key = internal::Key_code::R, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT | internal::Modifier::CONTROL },
    "Decrease rotation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.clipping_plane_->decrease_rotation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  // Double-click

  action_registry_->register_action(
    section_name,
    "align_cp_to_camera",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::LEFT, .action = internal::Action::PRESS, .mods = internal::Modifier::CONTROL, .double_click = true },
    "Align clipping plane to camera",
    [](internal::Event_context& ctx) {
      ctx.viewer.clipping_plane_->align_to_direction(ctx.viewer.camera_->get_forward());
      return true; // need to redraw
    }
  );
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_application_actions() const {

  const std::string& section_name = "Application";

  action_registry_->register_action(
    section_name,
    "exit",
    internal::Key_binding{ .key = internal::Key_code::ESCAPE, .action = internal::Action::RELEASE },
    "Exit program",
    [](internal::Event_context& ctx) { 
      ctx.viewer.window_->should_close(GLFW_TRUE); 
      return false; // don't need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_print_state",
    internal::Key_binding{ .key = internal::Key_code::T, .action = internal::Action::RELEASE },
    "Toggle application state refresh (FPS, etc.)",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_print_application_state(); 
      return false; // don't need to redraw
    }
  );
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_window_actions() const {

  const std::string& section_name = "Window";

  action_registry_->register_action(
    section_name,
    "toggle_fullscreen",
    internal::Key_binding{ .key = internal::Key_code::ENTER, .action = internal::Action::RELEASE, .mods = internal::Modifier::ALT },
    "Switch windowed/fullscreen",
    [](internal::Event_context& ctx) { 
      ctx.viewer.window_->toggle_fullscreen(); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "screenshot",
    internal::Key_binding{ .key = internal::Key_code::F2, .action = internal::Action::RELEASE },
    "Save a screenshot of the current view",
    [](internal::Event_context& ctx) { 
      ctx.viewer.capture_screenshot("./screenshot.png"); 
      return false; // don't need to redraw
    }
  );
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_animation_actions() const {

  const std::string& section_name = "Animation_controller";

  action_registry_->register_action(
    section_name,
    "clear",
    internal::Key_binding{
      .key = internal::Key_code::F1,
      .action = internal::Action::RELEASE,
      .mods = internal::Modifier::CONTROL | internal::Modifier::SHIFT },
    "Delete animation key frames (only available in orbiter)",
    [](internal::Event_context& ctx) {
      ctx.viewer.animation_controller_->clear_buffer();
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "save_key_frame",
    internal::Key_binding{
      .key = internal::Key_code::F1,
      .action = internal::Action::RELEASE,
      .mods = internal::Modifier::SHIFT },
    "Add a key frame to animation (only available in orbiter)",
    [](internal::Event_context& ctx) { 
      ctx.viewer.save_key_frame(); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "run_or_stop",
    internal::Key_binding{
      .key = internal::Key_code::F1,
      .action = internal::Action::RELEASE },
    "Run or stop the animation",
    [](internal::Event_context& ctx) { 
      ctx.viewer.run_or_stop_animation(); 
      return true; // need to redraw
    });
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_camera_actions() const {

  const std::string& section_name = "Camera";

  action_registry_->register_action(
    section_name,
    "change_pivot_point",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::RIGHT, .action = internal::Action::RELEASE, .mods = internal::Modifier::SHIFT },
    "Change orbit pivot while dragging",
    [](internal::Event_context& ctx) { 
      ctx.viewer.change_pivot_point(); 
      return false; // don't need to redraw
    });

  // Movement (hold)

  action_registry_->register_action(
    section_name,
    "move_up",
    internal::Key_binding{ .key = internal::Key_code::UP, .action = internal::Action::HOLD },
    "Move camera up",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->move_up(ctx.dt); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "move_down",
    internal::Key_binding{ .key = internal::Key_code::DOWN, .action = internal::Action::HOLD },
    "Move camera down",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->move_down(ctx.dt); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "move_left",
    internal::Key_binding{ .key = internal::Key_code::LEFT, .action = internal::Action::HOLD },
    "Move camera left",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->move_left(ctx.dt); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "move_right",
    internal::Key_binding{ .key = internal::Key_code::RIGHT, .action = internal::Action::HOLD },
    "Move camera right",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->move_right(ctx.dt); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "move_forward",
    internal::Key_binding{ .key = internal::Key_code::UP, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Move camera forward",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->move(ctx.dt); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "move_backwards",
    internal::Key_binding{ .key = internal::Key_code::DOWN, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Move camera backwards",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->move(-ctx.dt); 
      return true; // need to redraw
    });

  // Mode / type / constraint

  action_registry_->register_action(
    section_name,
    "switch_type",
    internal::Key_binding{ .key = internal::Key_code::SPACE, .action = internal::Action::RELEASE },
    "Switch Orbiter / Free-fly",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->toggle_type(); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "switch_mode",
    internal::Key_binding{ .key = internal::Key_code::O, .action = internal::Action::RELEASE },
    "Switch Perspective / Orthographic",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->toggle_mode(); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "switch_constraint_axis",
    internal::Key_binding{ .key = internal::Key_code::A, .action = internal::Action::RELEASE, .mods = internal::Modifier::ALT },
    "Cycle rotation constraint axis",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->switch_constraint_axis(); 
      return false; // don't need to redraw
    });

  // Speed (hold)

  action_registry_->register_action(
    section_name,
    "inc_translation_speed",
    internal::Key_binding{ .key = internal::Key_code::X, .action = internal::Action::HOLD },
    "Increase translation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->increase_translation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "dec_translation_speed",
    internal::Key_binding{ .key = internal::Key_code::X, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Decrease translation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->decrease_translation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "inc_rotation_speed",
    internal::Key_binding{ .key = internal::Key_code::R, .action = internal::Action::HOLD },
    "Increase rotation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->increase_rotation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  action_registry_->register_action(
    section_name,
    "dec_rotation_speed",
    internal::Key_binding{ .key = internal::Key_code::R, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Decrease rotation speed",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->decrease_rotation_speed(ctx.dt); 
      return false; // don't need to redraw
    });

  // Mouse drags (hold)

  action_registry_->register_action(
    section_name,
    "rotate",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::LEFT, .action = internal::Action::HOLD },
    "Rotate camera (drag)",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Drag_event>(ctx.event);
      ctx.viewer.camera_->rotation(e.dx, e.dy);
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "translate",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::RIGHT, .action = internal::Action::HOLD },
    "Translate camera (drag)",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Drag_event>(ctx.event);
      ctx.viewer.camera_->translation(
        -e.dx * CGAL_CAMERA_DRAG_TRANSLATION_SPEED,
         e.dy * CGAL_CAMERA_DRAG_TRANSLATION_SPEED);
      return true; // need to redraw
    });

  // Scroll

  action_registry_->register_action(
    section_name,
    "zoom",
    internal::Scroll_binding{},
    "Zoom in/out",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Scroll_event>(ctx.event);
      ctx.viewer.zoom_camera(static_cast<float>(e.yoffset));
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "fov",
    internal::Scroll_binding{ .mods = internal::Modifier::ALT },
    "Adjust FOV when perspective",
    [](internal::Event_context& ctx) {
      const auto& e = std::get<internal::Scroll_event>(ctx.event);
      ctx.viewer.change_camera_fov(static_cast<float>(e.yoffset));
      return true; // need to redraw
    });

  // Double clicks

  action_registry_->register_action(
    section_name,
    "align_nearest_axis",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::LEFT, .action = internal::Action::PRESS, .double_click = true },
    "Align camera to nearest axis",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->align_to_nearest_axis(); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "align_to_clipping_plane",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::LEFT, .action = internal::Action::PRESS, .mods = internal::Modifier::SHIFT, .double_click = true },
    "Align camera to clipping plane",
    [](internal::Event_context& ctx) {
      ctx.viewer.camera_->align_to_plane(ctx.viewer.clipping_plane_->get_normal());
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "reset_position",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::RIGHT, .action = internal::Action::PRESS, .double_click = true },
    "Re-center camera on bounding box",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->reset_position(); 
      return true; // need to redraw
    });

  action_registry_->register_action(
    section_name,
    "reset_position_and_orientation",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::RIGHT, .action = internal::Action::PRESS, .mods = internal::Modifier::CONTROL, .double_click = true },
    "Reset camera position and orientation",
    [](internal::Event_context& ctx) { 
      ctx.viewer.reset_camera(); 
      return true; // need to redraw
    });
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_scene_actions() const {

  const std::string& section_name = "Scene";

  action_registry_->register_action(
    section_name,
    "toggle_face_normals",
    internal::Key_binding{ .key = internal::Key_code::N, .action = internal::Action::RELEASE, .mods = internal::Modifier::SHIFT },
    "Toggle face vs. vertex normals for normal display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_display_face_normal(); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_normals_display",
    internal::Key_binding{ .key = internal::Key_code::N, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL },
    "Toggle normals display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_normals();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_normals_mono_color",
    internal::Key_binding{ .key = internal::Key_code::M, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL },
    "Toggle normals mono color",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_use_mono_color_normals();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_triangles_display",
    internal::Key_binding{ .key = internal::Key_code::T, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL },
    "Toggle triangles display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_mesh_triangles();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_world_axis",
    internal::Key_binding{ .key = internal::Key_code::A, .action = internal::Action::RELEASE },
    "Toggle world axis display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_world_axis();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_xy_grid",
    internal::Key_binding{ .key = internal::Key_code::G, .action = internal::Action::RELEASE },
    "Toggle XY grid display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_xy_grid();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_faces",
    internal::Key_binding{ .key = internal::Key_code::W, .action = internal::Action::RELEASE },
    "Toggle faces display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_faces();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_vertices",
    internal::Key_binding{ .key = internal::Key_code::V, .action = internal::Action::RELEASE },
    "Toggle vertices display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_vertices();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_edges",
    internal::Key_binding{ .key = internal::Key_code::E, .action = internal::Action::RELEASE },
    "Toggle edges display",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_edges();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_shading",
    internal::Key_binding{ .key = internal::Key_code::S, .action = internal::Action::RELEASE },
    "Switch flat / Gouraud shading",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_flat_shading();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_inverse_normal",
    internal::Key_binding{ .key = internal::Key_code::N, .action = internal::Action::RELEASE },
    "Invert direction of all normals",
    [](internal::Event_context& ctx) { 
      ctx.viewer.reverse_all_normals();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_mono_color",
    internal::Key_binding{ .key = internal::Key_code::M, .action = internal::Action::RELEASE },
    "Toggle mono color",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_use_mono_color();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_cylinder_edge",
    internal::Key_binding{ .key = internal::Key_code::E, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL },
    "Display edges as cylinders",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_cylinder_edge();
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "toggle_sphere_vertex",
    internal::Key_binding{ .key = internal::Key_code::V, .action = internal::Action::RELEASE, .mods = internal::Modifier::CONTROL },
    "Display vertices as spheres",
    [](internal::Event_context& ctx) { 
      ctx.viewer.toggle_draw_sphere_vertex();
      return true; // need to redraw
    }
  );

  // Section: Scene - size (hold to repeat)

  action_registry_->register_action(
    section_name,
    "inc_points_size",
    internal::Key_binding{ .key = internal::Key_code::EQUAL, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL | internal::Modifier::SHIFT },
    "Increase vertex size",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_size_vertex(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "inc_points_size_kp",
    internal::Key_binding{ .key = internal::Key_code::KP_ADD, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Increase vertex size (numpad)",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_size_vertex(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_points_size",
    internal::Key_binding{ .key = internal::Key_code::KEY_6, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Decrease vertex size",
    [](internal::Event_context& ctx) { 
      ctx.viewer.decrease_size_vertex(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_points_size_kp",
    internal::Key_binding{ .key = internal::Key_code::KP_SUBTRACT, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Decrease vertex size (numpad)",
    [](internal::Event_context& ctx) { 
      ctx.viewer.decrease_size_vertex(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "inc_edges_size",
    internal::Key_binding{ .key = internal::Key_code::EQUAL, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Increase edge size",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_size_edge(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "inc_edges_size_kp",
    internal::Key_binding{ .key = internal::Key_code::KP_ADD, .action = internal::Action::HOLD },
    "Increase edge size (numpad)",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_size_edge(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_edges_size",
    internal::Key_binding{ .key = internal::Key_code::KEY_6, .action = internal::Action::HOLD },
    "Decrease edge size",
    [](internal::Event_context& ctx) { 
      ctx.viewer.decrease_size_edge(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_edges_size_kp",
    internal::Key_binding{ .key = internal::Key_code::KP_SUBTRACT, .action = internal::Action::HOLD },
    "Decrease edge size (numpad)",
    [](internal::Event_context& ctx) { 
      ctx.viewer.decrease_size_edge(ctx.dt); 
      return true; // need to redraw
    }
  );

  // Section: Scene - reset

  action_registry_->register_action(
    section_name,
    "reset_camera_and_cp",
    internal::Key_binding{ .key = internal::Key_code::R, .action = internal::Action::RELEASE, .mods = internal::Modifier::ALT },
    "Reset camera and clipping plane",
    [](internal::Event_context& ctx) { 
      ctx.viewer.reset_camera_and_clipping_plane(); 
      return true; // need to redraw
    }
  );

  // Section: Scene - double-click event

  action_registry_->register_action(
    section_name,
    "show_entire_scene",
    internal::Mouse_btn_binding{ .button = internal::Mouse_button::MIDDLE, .action = internal::Action::PRESS, .double_click = true },
    "Re-fit scene to the viewport",
    [](internal::Event_context& ctx) { 
      ctx.viewer.camera_->reset_size(); 
      return true; // need to redraw
    }
  );
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_light_actions() const {

  const std::string& section_name = "Scene";

  action_registry_->register_action(
    section_name,
    "inc_light_all",
    internal::Key_binding{ .key = internal::Key_code::PAGE_UP, .action = internal::Action::HOLD },
    "Increase all-channel light",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_light_all(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_light_all",
    internal::Key_binding{ .key = internal::Key_code::PAGE_DOWN, .action = internal::Action::HOLD },
    "Decrease all-channel light",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_light_all(-ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "inc_light_r",
    internal::Key_binding{ .key = internal::Key_code::PAGE_UP, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Increase red channel",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_red_component(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_light_r",
    internal::Key_binding{ .key = internal::Key_code::PAGE_DOWN, .action = internal::Action::HOLD, .mods = internal::Modifier::SHIFT },
    "Decrease red channel",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_red_component(-ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "inc_light_g",
    internal::Key_binding{ .key = internal::Key_code::PAGE_UP, .action = internal::Action::HOLD, .mods = internal::Modifier::ALT },
    "Increase green channel",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_green_component(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_light_g",
    internal::Key_binding{ .key = internal::Key_code::PAGE_DOWN, .action = internal::Action::HOLD, .mods = internal::Modifier::ALT },
    "Decrease green channel",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_green_component(-ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "inc_light_b",
    internal::Key_binding{ .key = internal::Key_code::PAGE_UP, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Increase blue channel",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_blue_component(ctx.dt); 
      return true; // need to redraw
    }
  );

  action_registry_->register_action(
    section_name,
    "dec_light_b",
    internal::Key_binding{ .key = internal::Key_code::PAGE_DOWN, .action = internal::Action::HOLD, .mods = internal::Modifier::CONTROL },
    "Decrease blue channel",
    [](internal::Event_context& ctx) { 
      ctx.viewer.increase_blue_component(-ctx.dt); 
      return true; // need to redraw
    }
  );
}

} // namespace GLFW
} // namespace CGAL


#endif // CGAL_GLFW_INTERNAL_BASIC_VIEWER_ACTIONS_SPLIT_H
