#ifndef CGAL_GLFW_INTERNAL_ORBITER_CAMERA_IMPL_H
#define CGAL_GLFW_INTERNAL_ORBITER_CAMERA_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Orbiter_camera.h"

#else 
#define CGAL_INLINE_FUNCTION 
#endif

#include <cmath>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Orbiter_camera::Orbiter_camera(float fov) : fov_(fov) {}

CGAL_INLINE_FUNCTION
void Orbiter_camera::lookat(const vec3f& target, float radius) {
  target_ = target; // camera focus
  default_target_ = target_; 
  distance_ = 2 * radius;
  default_distance_ = distance_;
  target_distance_ = distance_;
  radius_ = radius; 
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::lookat(const vec3f& pmin, const vec3f& pmax) {
  lookat(utils::center(pmin, pmax), utils::distance(pmin, pmax));
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::begin_drag(float nx, float ny) {
  if (animating_align_) return; 

  dragging_ = true; 
  q_at_grab_ = orientation_; 
  p1_at_grab_ = project_to_sphere(nx, ny); 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::on_drag(float nx, float ny) {
  if (!dragging_) return;

  vec3f p2 = project_to_sphere(nx, ny);
  
  vec3f axis = p1_at_grab_.cross(p2); 
  float w = p1_at_grab_.dot(p2); 
  quatf q_drag(w, axis.x(), axis.y(), axis.z()); 
  q_drag.normalize(); 

  q_drag = apply_constraint(q_drag, constraint_axis_);

  orientation_ = q_drag * q_at_grab_; 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::end_drag() {
  dragging_ = false; 
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::pan(float dx, float dy) {
  const float half_h = (mode_ == Mode::PERSPECTIVE) 
                       ? distance_ * std::tan(utils::radians(fov_) * 0.5) 
                       : distance_ * 0.5;

  translation_.x() -= dx * half_h; 
  translation_.y() -= dy * half_h; 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::move_up(float dt) {
  const float half_h = (mode_ == Mode::PERSPECTIVE) 
                       ? distance_ * std::tan(utils::radians(fov_) * 0.5) 
                       : distance_ * 0.5;

  translation_.y() += dt * half_h; 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::move_down(float dt) {
  const float half_h = (mode_ == Mode::PERSPECTIVE) 
                       ? distance_ * std::tan(utils::radians(fov_) * 0.5) 
                       : distance_ * 0.5;

  translation_.y() -= dt * half_h; 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::move_left(float dt) {
  const float half_h = (mode_ == Mode::PERSPECTIVE) 
                       ? distance_ * std::tan(utils::radians(fov_) * 0.5) 
                       : distance_ * 0.5;

  translation_.x() -= dt * half_h; 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::move_right(float dt) {
  const float half_h = (mode_ == Mode::PERSPECTIVE) 
                       ? distance_ * std::tan(utils::radians(fov_) * 0.5) 
                       : distance_ * 0.5;

  translation_.x() += dt * half_h; 
} 


CGAL_INLINE_FUNCTION
void Orbiter_camera::zoom(float z) {
  target_distance_ *= std::exp(-z); 
  target_distance_ = std::max(target_distance_, 0.01f * radius_); 
} 

CGAL_INLINE_FUNCTION
void Orbiter_camera::update(float dt) {
  distance_ += (1.0f - std::exp(-zoom_smooth_ * dt)) * (target_distance_ - distance_); 

  if (animating_align_) {
    align_t_ = std::min(1.0f, align_t_ + dt / align_duration_); 
    const float s = align_t_ * align_t_ * (3.0f - 2.0f * align_t_); // cubic smoothstep
    orientation_ = orientation_start_.slerp(s, orientation_target_).normalized();
    if (align_t_ >= 1.0f) animating_align_ = false; 
  }
}

CGAL_INLINE_FUNCTION
mat4f Orbiter_camera::view() const {
  const mat4f R = transform::rotation(orientation_.conjugate()); 
  const mat4f T = transform::translation(position());
  return (T * R).inverse();
} 

CGAL_INLINE_FUNCTION
mat4f Orbiter_camera::proj(float aspect) const {
  float znear = std::max(0.1f, distance_ - 2 * radius_); 
  float zfar = std::max(100.0f, distance_ + 2 * radius_); 

  if (mode_ == Mode::ORTHOGRAPHIC) {
    float half_w = distance_ * aspect * 0.5f;
    float half_h = distance_ * 0.5f;
    return utils::ortho(-half_w, half_w, -half_h, half_h, znear, zfar);
  }

  return utils::perspective(utils::radians(fov_), aspect, znear, zfar);
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::align_to_nearest_axis() {
  quatf target = compute_aligned_to_nearest_axis();
  begin_orientation_animation(target);
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::align_to_plane(const vec3f& n) {
  quatf target = compute_aligned_to_plane(n); 
  begin_orientation_animation(target);
}

CGAL_INLINE_FUNCTION
vec3f Orbiter_camera::position() const { 
  vec3f p = target_ - forward() * distance_;
  p += right() * translation_.x();  
  p += up() * translation_.y();  
  return p; 
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::increase_fov(float f) { 
  if (mode_ == Mode::ORTHOGRAPHIC) {
    fov_ = std::clamp(fov_ + f * 2.f, 25.0f, 105.0f); 
    return; 
  }

  const float old_tan = std::tan(utils::radians(fov_) * 0.5); 
  fov_ = std::clamp(fov_ + f * 2.f, 25.0f, 105.0f); 
  const float new_tan = std::tan(utils::radians(fov_) * 0.5); 

  const float factor = old_tan / new_tan; 
  distance_ *= factor; 
  target_distance_ *= factor;  
}

CGAL_INLINE_FUNCTION
std::string Orbiter_camera::constraint_axis_str() const {
  switch (constraint_axis_) {
    case Constraint_axis::FORWARD_AXIS: return "Forward"; 
    case Constraint_axis::RIGHT_AXIS: return "Right"; 
    case Constraint_axis::UP_AXIS: return "Up"; 
    default: return "None"; 
  }
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::orientation(const vec3f& direction, float up_angle) { 
  orientation_ = quatf::FromTwoVectors(-vec3f::UnitZ(), direction.normalized()).inverse();
  orientation_ *= quatf(Eigen::AngleAxisf(utils::radians(up_angle), forward())); 
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::switch_constraint_axis() {
  constraint_axis_ = static_cast<Constraint_axis>(
    (static_cast<int>(constraint_axis_) + 1) % static_cast<int>(Constraint_axis::NB_ELT)); 
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::change_pivot_point(float nx, float ny) {
  const float half_h = (mode_ == Mode::PERSPECTIVE)
                       ? distance_ * tan(utils::radians(fov_) * 0.5f)
                       : distance_ * 0.5f; 

  // World point that currently projects to (nx, ny) on the focal plane.
  target_ = position() + forward() * distance_;
  target_ += right() * (nx * half_h);   
  target_ += up() * (ny * half_h);
  
  translation_.x() -= nx * half_h; 
  translation_.y() -= ny * half_h; 
}

CGAL_INLINE_FUNCTION
quatf Orbiter_camera::compute_aligned_to_nearest_axis() const {
  const vec3f world_forward = forward(); 
  const vec3f world_up = up();
  
  vec3f f_snap = snap_to_principal_axis(world_forward); 
  vec3f u_snap = snap_to_principal_axis(world_up);
  
  // Handle the case where f_snap == u_snap
  if (std::abs(f_snap.dot(u_snap)) > 0.5f) {
    // Remove the f_snap component from world_up so re-snap lands on a perpendicular axis.
    const vec3f up_residual = world_up - world_up.dot(f_snap) * f_snap;
    u_snap = snap_to_principal_axis(up_residual); 
  }

  const vec3f r_snap = u_snap.cross(-f_snap); 

  mat3f c2w_mat; // camera-to-world matrix
  c2w_mat.col(0) = r_snap; 
  c2w_mat.col(1) = u_snap; 
  c2w_mat.col(2) = -f_snap; 

  return quatf(c2w_mat.transpose()).normalized(); // world-to-camera matrix
}

CGAL_INLINE_FUNCTION
quatf Orbiter_camera::compute_aligned_to_plane(const vec3f& n) const {
  const vec3f f = forward();
  const vec3f target_dir = (f.dot(-n) > f.dot(n)) ? -n : n;

  const quatf R = quatf::FromTwoVectors(f, target_dir);
  return (orientation_ * R.conjugate()).normalized(); 
}

CGAL_INLINE_FUNCTION
void Orbiter_camera::begin_orientation_animation(const quatf& target) {
  orientation_start_ = orientation_; 
  orientation_target_ = target; 

  if (orientation_start_.dot(orientation_target_) < 0.0f) {
    orientation_target_.coeffs() *= -1.0f; 
  }

  align_t_ = 0.0f; 
  animating_align_ = true; 
}

CGAL_INLINE_FUNCTION
quatf Orbiter_camera::apply_constraint(const quatf& q, Orbiter_camera::Constraint_axis c) {
  if (c == Constraint_axis::NO_CONSTRAINT) return q; 

  vec3f forced_axis = constraint_axis_vec3f(c); 

  const vec3f v(q.x(), q.y(), q.z()); 
  const vec3f p = v.dot(forced_axis) * forced_axis; 

  quatf twist(q.w(), p.x(), p.y(), p.z()); 

  const float n = twist.norm(); 
  if (n < 1e-6f) return quatf::Identity(); 
 
  return twist.normalized(); 
}

CGAL_INLINE_FUNCTION
vec3f Orbiter_camera::constraint_axis_vec3f(Constraint_axis c) {
  switch(c) {
    case Constraint_axis::FORWARD_AXIS:
      return -vec3f::UnitZ();  
    case Constraint_axis::RIGHT_AXIS: 
      return vec3f::UnitX();  
    case Constraint_axis::UP_AXIS:
      return vec3f::UnitY();  
    default: // Constraint_axis::NO_CONSTRAINT
      return vec3f::Zero();   
  }
}

CGAL_INLINE_FUNCTION
vec3f Orbiter_camera::project_to_sphere(float nx, float ny) {
  // Arcball Shoemake formula
  float r2 = nx * nx + ny * ny;
  if (r2 <= 1.0f) {
    float nz = std::sqrt(1.0f - r2); 
    return vec3f(nx, ny, nz);
  } 

  float inv_r = 1.0f / std::sqrt(r2);
  return vec3f(nx * inv_r, ny * inv_r, 0);
}

CGAL_INLINE_FUNCTION
vec3f Orbiter_camera::snap_to_principal_axis(const vec3f& v) {
  Eigen::Index i; 
  v.cwiseAbs().maxCoeff(&i); 
  vec3f r = vec3f::Zero(); 
  r[i] = std::copysign(1.0f, v[i]); 
  return r; 
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_ORBITER_CAMERA_IMPL_H
