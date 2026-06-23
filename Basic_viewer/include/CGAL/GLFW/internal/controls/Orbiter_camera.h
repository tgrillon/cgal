#ifndef CGAL_GLFW_INTERNAL_ORBITER_CAMERA_H
#define CGAL_GLFW_INTERNAL_ORBITER_CAMERA_H

#include <CGAL/GLFW/internal/math/math_utils.h>
#include <string>

namespace CGAL {
namespace GLFW {
namespace internal {

class Orbiter_camera {
public: 
  enum class Mode { PERSPECTIVE, ORTHOGRAPHIC }; 
  enum class Constraint_axis { NO_CONSTRAINT = 0, RIGHT_AXIS, UP_AXIS, FORWARD_AXIS, NB_ELT };

public: // Ctors and dtor 
  explicit Orbiter_camera(float fov=45.0f); 
  
  Orbiter_camera(const Orbiter_camera&) = delete; 
  Orbiter_camera(Orbiter_camera&&) = default; 

  Orbiter_camera& operator=(const Orbiter_camera&) = delete; 
  Orbiter_camera& operator=(Orbiter_camera&&) = default;
  
  ~Orbiter_camera() = default;

public: 
  void lookat(const vec3f& target, float radius); 
  void lookat(const vec3f& pmin, const vec3f& pmax); 

  void begin_drag(float nx, float ny); 
  void on_drag(float nx, float ny); 
  void end_drag(); 

  // dx and dy should be pre-multiply by the viewport aspect ratio
  void pan(float dx, float dy); 
  void move_up(float dt); 
  void move_down(float dt); 
  void move_left(float dt); 
  void move_right(float dt); 

  void zoom(float z); 
  void update(float dt);

  bool need_update() const { return animating_align_ || std::abs(target_distance_ - distance_) > 1e-3f; }

  mat4f view() const; 
  mat4f proj(float aspect) const;

  void align_to_nearest_axis(); 
  void align_to_plane(const vec3f& normal); 

  void reset_position() { translation_ = vec2f::Zero(); }
  void reset_orientation() { orientation_ = default_orientation_; }
  void reset_target() { target_ = default_target_; }
  void reset_distance() { distance_ = default_distance_; } 
  void reset_all() { reset_orientation(); reset_position(); reset_target(); reset_distance(); } 

public: // accessors
  vec3f forward() const { return (orientation_.conjugate() * -vec3f::UnitZ()).normalized(); } 
  vec3f right() const { return (orientation_.conjugate() * vec3f::UnitX()).normalized(); }
  vec3f up() const { return (orientation_.conjugate() * vec3f::UnitY()).normalized(); }  
  
  vec3f position() const;  
  
  const quatf& orientation() const { return orientation_; }  
  const vec3f& target() const { return target_; }  
  float distance() const { return distance_; }  
  float radius() const { return radius_; }  
  float fov() const { return fov_; }  

  bool is_orthographic() const { return mode_ == Mode::ORTHOGRAPHIC; } 
  
  std::string constraint_axis_str() const; 

public: // setters
  void orientation(const quatf& q) { orientation_ = q; }
  void orientation(const vec3f& direction, float up_angle);
  void target(const vec3f& t) { target_ = t; }
  void radius(float r) { radius_ = r; }
  void distance(float d) { distance_ = d; }

  void mode(Mode m) { mode_ = m; }
  void toggle_mode() { mode_ = (mode_ == Mode::ORTHOGRAPHIC) ? Mode::PERSPECTIVE : Mode::ORTHOGRAPHIC; }

  void increase_fov(float f);

  void disable_smoothness() { zoom_smooth_ = 1e6f; }   
  void increase_zoom_smoothness(float dt) { zoom_smooth_ = std::max(zoom_smooth_ - dt * 4.0f, 1.0f); }

  void switch_constraint_axis(); 
  void constraint_axis(Constraint_axis c) { constraint_axis_ = c; }

  void change_pivot_point(float nx, float ny); 

private: 
  quatf compute_aligned_to_nearest_axis() const;
  quatf compute_aligned_to_plane(const vec3f& n) const;
  void begin_orientation_animation(const quatf& target); 

private: 
  static vec3f project_to_sphere(float nx, float ny);

  static quatf apply_constraint(const quatf& q, Constraint_axis c);
  static vec3f constraint_axis_vec3f(Constraint_axis c);

  static vec3f snap_to_principal_axis(const vec3f& v); 

private: 
  quatf orientation_{quatf::Identity()}; 
  vec3f target_{vec3f::Zero()}; // focused world point
  vec2f translation_{vec2f::Zero()}; // camera translation
  float distance_{0.0f}; // Camera to target distance (zoom)
  float radius_{0.0f}; // scene radius to compute znear and zfar
  
  vec2f prev_pivot_pos{vec2f::Zero()}; 

  quatf default_orientation_{quatf::Identity()}; 
  vec3f default_target_{vec3f::Zero()}; 
  float default_distance_{0.0f}; 
  
  float fov_{45.0f}; 
  
  Mode mode_{Mode::PERSPECTIVE}; // Perspective or Orthographic 
  
  float target_distance_{0.0f};
  float zoom_smooth_{8.0f};
  
  bool dragging_{false}; 
  quatf q_at_grab_{quatf::Identity()};
  vec3f p1_at_grab_{vec3f::Zero()};

  quatf orientation_start_{quatf::Identity()}; 
  quatf orientation_target_{quatf::Identity()};
  float align_t_{1.0f};  
  float align_duration_{0.250f}; // seconds  
  bool animating_align_{false};   

  Constraint_axis constraint_axis_{Constraint_axis::NO_CONSTRAINT}; 
};


} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Orbiter_camera_impl.h"
#endif

#endif // CGAL_GLFW_INTERNAL_ORBITER_CAMERA_H
