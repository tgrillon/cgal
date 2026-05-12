#ifndef CGAL_GLFW_BASIC_VIEWER_IMPL_H
#define CGAL_GLFW_BASIC_VIEWER_IMPL_H

#include "Basic_viewer.h"
#include <CGAL/basic.h>
#include <CGAL/config.h>
#include <CGAL/utils_classes.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include <CGAL/GLFW/Basic_viewer.h>

#else
#define CGAL_INLINE_FUNCTION
#endif

namespace CGAL {
namespace GLFW {

CGAL_INLINE_FUNCTION
Basic_viewer::Basic_viewer(const Graphics_scene &scene,
                           const char *title, 
                           const Basic_viewer_options& opts)
    : window_({ .title=title, .hidden=opts.hidden_window }),
      scene_(scene), camera_(), clipping_plane_(), 
      renderer_(scene_, camera_, clipping_plane_, window_)
{
  renderer_.draw_vertices(opts.draw_vertices),
  renderer_.draw_edges(opts.draw_edges), 
  renderer_.draw_faces(opts.draw_faces),
  renderer_.draw_rays(opts.draw_rays), 
  renderer_.draw_lines(opts.draw_lines),
  renderer_.use_mono_color(opts.use_mono_color),
  renderer_.reverse_normal(opts.inverse_normal), 
  renderer_.flat_shading(opts.flat_shading), 

  setup_context(); 
}

CGAL_INLINE_FUNCTION
void Basic_viewer::setup_context() {
  if (!window_.is_valid()) return;

  initialize_camera();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::setup_inputs() {
  action_registry_.emplace();
  animation_controller_.emplace();

  register_actions();

  auto callback = [this](const internal::Event& event) {
    internal::Event_context context{ .viewer=*this, .event=event };
    if (action_registry_->dispatch(context)) {
      need_update_ = true;
    } 
  };

  window_.on_key(callback);
  window_.on_mouse_btn(callback);
  window_.on_scroll(callback);

  window_.on_resize([this](const internal::Resize_event& event) {
    glViewport(0, 0, event.width, event.height);
    need_update_ = true;
  });

  window_.on_cursor_move([this](const internal::Cursor_event& e) {
    float dx = (e.xpos - last_x_) / window_.aspect_ratio();                                                
    float dy = (e.ypos - last_y_) / window_.aspect_ratio();                                                
    last_x_ = e.xpos;
    last_y_ = e.ypos;                                                                             
                                                                                                  
    auto mods = internal::Input::active_modifiers(window_.handle());
    action_registry_->for_each_hold<internal::Mouse_btn_binding>(                                 
      [&](const internal::Mouse_btn_binding& b) -> bool {                                         
        if (internal::Input::is_mouse_button_pressed(window_.handle(), b.button) && mods == b.mods) {
          internal::Drag_event ev{ b.button, b.mods, dx, dy };                  
          internal::Event_context ctx{ .viewer=*this, .event=ev };                            
          if (action_registry_->dispatch(ctx)) {
            need_update_ = true;             
          } 

          return true;                
        }
        return false;               
      });
  });

  action_registry_->print_help();
}

void Basic_viewer::render_scene(float dt) {
  // tick (mutates camera_, clipping_plane_)
  camera_.update(dt);
  clipping_plane_.update(dt);
  if (animation_controller_.has_value() && animation_controller_->is_running()) {
    auto kf = animation_controller_->run();
    camera_.set_orientation(kf.orientation);
    camera_.set_position(kf.position);
  }

  // pure render
  renderer_.render();
  window_.update();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::show() {
  setup_inputs();

  float elapsed_time = 0.0f;
  float last_frame = 0.0;
  while (!window_.should_close()) {
    float current_frame = static_cast<float>(glfwGetTime());
    float dt = current_frame - last_frame;
    last_frame = current_frame;
    if (dt < 1e-3)
      dt = 1e-3;

    handle_events(dt);
    if (need_update()) {
      render_scene(dt);
      need_update_ = false;
    }
    print_application_state(elapsed_time, dt);
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::make_screenshot(const std::string &filepath) {
  camera_.disable_smoothness();

  const int w = window_.framebuffer_width();
  const int h = window_.framebuffer_height();

  GLuint fbo, color_rbo, depth_rbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenRenderbuffers(1, &color_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, color_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, color_rbo);

  glGenRenderbuffers(1, &depth_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth_rbo);

  glViewport(0, 0, w, h);
  renderer_.render();

  const GLsizei N = 4;
  GLsizei stride = N * w + ((N * w) % 4 ? 4 - (N * w) % 4 : 0);
  std::vector<unsigned char> buffer(stride * h);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

  stbi_flip_vertically_on_write(true);
  if (!stbi_write_png(filepath.data(), w, h, N, buffer.data(), stride)) {
    std::cerr << "Failed to write screenshot to " << filepath << '\n';
  } else {
    std::cout << "Screenshot saved to " << filepath << "\n";
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteRenderbuffers(1, &color_rbo);
  glDeleteRenderbuffers(1, &depth_rbo);
  glDeleteFramebuffers(1, &fbo);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::initialize_camera() {
  vec3f pmin(scene_.bounding_box().xmin(), scene_.bounding_box().ymin(),
             scene_.bounding_box().zmin());

  vec3f pmax(scene_.bounding_box().xmax(), scene_.bounding_box().ymax(),
             scene_.bounding_box().zmax());

  bounding_box_ = {pmin, pmax};

  camera_.lookat(pmin, pmax);
  renderer_.scene_scale(camera_.get_radius()); 

  if (is_two_dimensional()) {
    camera_.set_constraint_axis(internal::Camera::Constraint_axis::FORWARD_AXIS);
    camera_.set_orthographic();
  }
}

CGAL_INLINE_FUNCTION
CGAL::Plane_3<Basic_viewer::Local_kernel> Basic_viewer::clipping_plane() const {
  mat4f cpm = clipping_plane_.model_matrix();
  CGAL::Aff_transformation_3<Basic_viewer::Local_kernel> aff(
      cpm(0, 0), cpm(0, 1), cpm(0, 2), cpm(0, 3), cpm(1, 0), cpm(1, 1),
      cpm(1, 2), cpm(1, 3), cpm(2, 0), cpm(2, 1), cpm(2, 2), cpm(2, 3));

  CGAL::Plane_3<Local_kernel> p3(0, 0, 1, 0);
  return p3.transform(aff);
}

CGAL_INLINE_FUNCTION
bool Basic_viewer::need_update() const {
  return camera_.need_update() 
      || clipping_plane_.need_update() 
      || animation_controller_->is_running() 
      || need_update_;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::handle_events(float dt) {
  glfwPollEvents();

  internal::Modifier mods = internal::Input::active_modifiers(window_.handle()); 
  action_registry_->for_each_hold<internal::Key_binding>([&](const internal::Key_binding& b) {
    if (internal::Input::is_key_pressed(window_.handle(), b.key) && mods == b.mods) {
      internal::Key_event event {.key=b.key, .scancode=0, .action=b.action, .mods=b.mods }; 
      internal::Event_context context { .viewer=*this, .event=event, .dt=dt};
      if (action_registry_->dispatch(context)) need_update_ = true; 
    } 
    return false; // don't stop at the first key hold trigger 
  }); 
}

CGAL_INLINE_FUNCTION
void Basic_viewer::print_application_state(float &elapsed_time, const float dt) {
  elapsed_time += dt;
  if (elapsed_time * 1000 > 100) // update terminal display each 100ms
  {
    elapsed_time = 0.0f;
    if (print_application_state_) {
      std::cout
          << "\33[2K" << std::round(1 / dt) << " fps    "
          << dt * 1000 << " ms\n\33[2K"
          << "Camera type: " << (camera_.is_orbiter() ? "ORBITER" : "FREE_FLY")
          << "    "
          << "Camera mode: "
          << (camera_.is_orthographic() ? "ORTHOGRAPHIC" : "PERSPECTIVE")
          << "    "
          << "FOV: " << camera_.get_fov() << " \n\33[2K"
          << "Camera translation speed: " << camera_.get_translation_speed()
          << "    "
          << "Camera rotation speed: "
          << std::round(camera_.get_rotation_speed()) << "    "
          << "Camera constraint axis: " << camera_.get_constraint_axis_str()
          << "\n\33[2K"
          << "CP translation speed: " << clipping_plane_.translation_speed()
          << "    "
          << "CP rotation speed: "
          << std::round(clipping_plane_.rotation_speed()) << "    "
          << "CP constraint axis: " << clipping_plane_.constraint_axis_str()
          << "\n\33[2K"
          << "Reversed normals : " << (renderer_.reverse_normal() ? "TRUE" : "FALSE")
          << "    "
          << "Face normal : " << (renderer_.display_face_normal() ? "TRUE" : "FALSE")
          << "    "
          << "Shading : " << (renderer_.flat_shading() ? "FLAT" : "SMOOTH") << "\n\33[2K"
          << "Draw faces: " << (renderer_.draw_faces() ? "TRUE" : "FALSE") << "    "
          << "Draw edges: " << (renderer_.draw_edges() ? "TRUE" : "FALSE") << "    "
          << "Draw vertices: " << (renderer_.draw_vertices() ? "TRUE" : "FALSE")
          << "\n\33[2K"
          << "Draw lines: " << (renderer_.draw_lines() ? "TRUE" : "FALSE") << "    "
          << "Draw rays: " << (renderer_.draw_rays() ? "TRUE" : "FALSE") << "    "
          << "Draw normals: " << (renderer_.draw_normals() ? "TRUE" : "FALSE")
          << "\n\33[2K"
          << "Draw edges as cylinder: "
          << (renderer_.draw_cylinder_edge() ? "TRUE" : "FALSE") << "    "
          << "Draw vertices as sphere: "
          << (renderer_.draw_sphere_vertex() ? "TRUE" : "FALSE") << "\n\33[2K"
          << "Use default color: " << (renderer_.use_mono_color() ? "TRUE" : "FALSE")
          << "    "
          << "Number of key frames: "
          << animation_controller_->number_of_key_frames() << "\n\33[2K"
          << "Size of vertices: " << renderer_.size_vertices()
          << "   Size of edges: " << renderer_.size_edges() << "    "
          << "\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\r"
          << std::flush;
    }
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::change_pivot_point() {
  auto [mouse_x, mouse_y] = internal::Input::mouse_position(window_.handle());
  int width, height;  
  window_.window_size(width, height);

  vec2f nc = internal::utils::normalized_coordinates({mouse_x, mouse_y}, width, height);

  vec3f camera_position = camera_.get_position();
  float camera_x = camera_position.x();
  float camera_y = camera_position.y();

  float x_value = nc.x() + camera_x;
  float y_value = nc.y() + camera_y;

  if (internal::utils::inside_bounding_box_2d(
          {x_value, y_value},
          {bounding_box_.first.x(), bounding_box_.first.y()},
          {bounding_box_.second.x(), bounding_box_.second.y()})) {
    std::cout << "INSIDE\n";
    camera_.set_center({nc.x(), nc.y(), 0});
  } else {
    camera_.set_center(
        internal::utils::center(bounding_box_.first, bounding_box_.second));
    std::cout << "OUTSIDE\n";
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::zoom_camera(float scroll_y) {
  float yoffset = scroll_y / window_.aspect_ratio();
  camera_.move(8.f * yoffset * CGAL_GLFW_CAMERA_ZOOM_SPEED);
  clipping_plane_.size(camera_.get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::change_camera_fov(float scroll_y) {
  if (camera_.is_orthographic()) return;
  float yoffset = scroll_y / window_.aspect_ratio();
  camera_.increase_fov(yoffset);
  clipping_plane_.size(camera_.get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::reset_camera() {
  camera_.reset_position();
  camera_.reset_orientation();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::reset_camera_and_clipping_plane() {
  camera_.reset_all();
  clipping_plane_.reset_all();
  clipping_plane_.size(camera_.get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::rotate_clipping_plane(float dx, float dy) {
  if (!clipping_plane_.clipping_enabled()) return;
  clipping_plane_.right_axis(camera_.get_right());
  clipping_plane_.up_axis(camera_.get_up());
  clipping_plane_.forward_axis(camera_.get_forward());
  clipping_plane_.rotation(dx, dy);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::translate_clipping_plane(float dx, float dy) {
  if (!clipping_plane_.clipping_enabled()) return;
  clipping_plane_.translation(
    -dx * CGAL_GLFW_CLIPPING_PLANE_DRAG_TRANSLATION_SPEED,
     dy * CGAL_GLFW_CLIPPING_PLANE_DRAG_TRANSLATION_SPEED);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::translate_clipping_plane_along_camera(float dx, float dy) {
  // Use whichever axis the user dragged most along, with the y-axis taking
  // precedence and inverted so that dragging up pushes the plane forward.
  float s = (std::fabs(dy) > std::fabs(dx)) ? -dy : dx;
  clipping_plane_.translation(camera_.get_forward(),
                               s * CGAL_GLFW_CLIPPING_PLANE_DRAG_TRANSLATION_SPEED);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::reset_clipping_plane() {
  clipping_plane_.reset_all();
  clipping_plane_.size(camera_.get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::save_key_frame() {
  if (!camera_.is_orbiter()) return;
  animation_controller_->add_key_frame(camera_.get_position(),
                                       camera_.get_orientation());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::run_or_stop_animation() {
  if (!camera_.is_orbiter()) return;
  if (animation_controller_->is_running()) {
    animation_controller_->stop(animation_controller_->get_frame());
  } else {
    animation_controller_->start();
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_actions() {
  if (!action_registry_.has_value()) return; 
  auto& registry = *action_registry_;
  register_clipping_plane_actions(registry);
  register_application_actions(registry);
  register_camera_actions(registry);
  register_window_actions(registry);
  register_scene_actions(registry);
  register_light_actions(registry);
  register_animation_actions(registry);
}

} // namespace GLFW
} // namespace CGAL

#include <CGAL/GLFW/internal/input/Basic_viewer_actions.inc.h>

#endif // CGAL_GLFW_BASIC_VIEWER_IMPL_H
