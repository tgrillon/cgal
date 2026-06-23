#ifndef CGAL_GLFW_INTERNAL_CAMERA_CONTROLLER_H
#define CGAL_GLFW_INTERNAL_CAMERA_CONTROLLER_H

#include "Orbiter_camera.h"

#include <CGAL/GLFW/internal/math/math_types.h>

namespace CGAL {
namespace GLFW {
namespace internal {

class Camera_controller {
public: // Ctors and dtor 
  explicit Camera_controller(float fov=45.0f) : orbiter_(fov) {}
  
  Camera_controller(const Camera_controller&) = delete; 
  Camera_controller(Camera_controller&&) = default; 

  Camera_controller& operator=(const Camera_controller&) = delete; 
  Camera_controller& operator=(Camera_controller&&) = default;
  
  ~Camera_controller() = default;

public: 
  void lookat(const vec3f& target, float radius) { scene_radius_ = radius; orbiter_.lookat(target, radius); }
  void lookat(const vec3f& pmin, const vec3f& pmax) { orbiter_.lookat(pmin, pmax); }
  void window_size(float w, float h) { width_ = w; height_ = h; }

  void update(float dt) { orbiter_.update(dt); }
  bool need_update() const { return orbiter_.need_update(); }

  mat4f view() const { return orbiter_.view(); }
  mat4f proj() const { return orbiter_.proj(aspect()); }

  void begin_drag(float xpos, float ypos) { orbiter_.begin_drag(to_ndc_x(xpos), to_ndc_y(ypos)); } 
  void on_drag(float xpos, float ypos) { orbiter_.on_drag(to_ndc_x(xpos), to_ndc_y(ypos)); } 
  void end_drag() { orbiter_.end_drag(); } 

  void pan(float dpx, float dpy) { auto d = pan_delta(dpx, dpy); orbiter_.pan(d.x(), d.y()); }
  void zoom(float z) { orbiter_.zoom(z); }

  void move_up(float dt) { orbiter_.move_up(dt); }
  void move_down(float dt) { orbiter_.move_down(dt); }
  void move_left(float dt) { orbiter_.move_left(dt); }
  void move_right(float dt) { orbiter_.move_right(dt); }

  void mode(Orbiter_camera::Mode m) { orbiter_.mode(m); }
  void toggle_mode() { orbiter_.toggle_mode(); }
  void set_orthographic() { orbiter_.mode(Orbiter_camera::Mode::ORTHOGRAPHIC); }
  bool is_orthographic() const { return orbiter_.is_orthographic(); }
  void increase_fov(float f) { orbiter_.increase_fov(f); }

  void disable_smoothness() { orbiter_.disable_smoothness(); }
  void increase_zoom_smoothness(float dt) { orbiter_.increase_zoom_smoothness(dt); }

  void align_to_nearest_axis() { orbiter_.align_to_nearest_axis(); }
  void align_to_plane(const vec3f& normal) { orbiter_.align_to_plane(normal); }

  void switch_constraint_axis() { orbiter_.switch_constraint_axis(); }
  std::string constraint_axis_str() const { return orbiter_.constraint_axis_str(); }

  void reset_all() { orbiter_.reset_all(); }
  void reset_orientation() { orbiter_.reset_orientation(); }
  void reset_position() { orbiter_.reset_position(); }
  void reset_target() { orbiter_.reset_target(); }
  void reset_distance() { orbiter_.reset_distance(); }

  vec3f position() const { return orbiter_.position(); }
  vec3f forward() const { return orbiter_.forward(); }
  vec3f right() const { return orbiter_.right(); }
  vec3f up() const { return orbiter_.up(); }
  const quatf& orientation() const { return orbiter_.orientation(); }
  vec3f target() const { return orbiter_.target(); }
  float distance() const { return orbiter_.distance(); }
  float fov() const { return orbiter_.fov(); }
  float radius() const { return scene_radius_; }
  
  void radius(float r) { orbiter_.radius(r); }
  void target(const vec3f& t) { orbiter_.target(t); }
  void position(const vec3f& p) { orbiter_.target(p + forward() * distance()); }
  void orientation(const quatf& o) { orbiter_.orientation(o); }
  void orientation(const vec3f& direction, float up_angle) { orbiter_.orientation(direction, up_angle); }
  void distance(float z) { orbiter_.distance(z); }
  void constraint_axis(Orbiter_camera::Constraint_axis c) { orbiter_.constraint_axis(c); }

  void change_pivot_point(float xpos, float ypos) { 
    const float nx = (2.0f * xpos - width_) / height_; 
    const float ny = (height_ - 2.0f * ypos) / height_; 
    orbiter_.change_pivot_point(nx, ny); 
  }

  Orbiter_camera& orbiter() { return orbiter_; }

  bool is_orbiter() const { return true; }

private:
  float aspect() const { return width_ / height_; } 

  float to_ndc_x(float px) const { return 2.0f * px / width_ - 1.0f; }
  float to_ndc_y(float py) const { return 1.0f - 2.0f * py / height_; }

  vec2f pan_delta(float dpx, float dpy) const { 
    return { 2.0f * dpx / height_, 2.0f * dpy / height_ };
  }

private: 
  Orbiter_camera orbiter_;
  float scene_radius_{1.0f}; 
  float width_{1.0f}; 
  float height_{1.0f}; 
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Camera_controller_impl.h"
#endif

#endif // CGAL_GLFW_INTERNAL_CAMERA_CONTROLLER_H
