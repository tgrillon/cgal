#ifndef CGAL_GLFW_INTERNAL_CLIPPING_PLANE_IMPL_H
#define CGAL_GLFW_INTERNAL_CLIPPING_PLANE_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Clipping_plane.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Clipping_plane::Clipping_plane(float size) {
  initialize_buffers();
  set_width(0.2f);
  add_line({0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0});
  constexpr int NB_SUBDIVISIONS = 30;
  generate_grid(size, NB_SUBDIVISIONS);
  load_buffers();
}

CGAL_INLINE_FUNCTION
void Clipping_plane::render() {
  if (render_clipping_plane_)
    draw(); 
}

CGAL_INLINE_FUNCTION
void Clipping_plane::update(const float delta_time) {
  if (need_update()) {
    float smooth_pitch = pitch_ + rotation_smooth_factor_ * (target_pitch_ - pitch_);
    float smooth_yaw = yaw_ + rotation_smooth_factor_ * (target_yaw_ - yaw_);

    float pitch_delta = utils::radians((smooth_pitch - pitch_) * rotation_speed_) * delta_time;
    float yaw_delta = utils::radians((smooth_yaw - yaw_) * rotation_speed_) * delta_time;

    quatf pitch_quaternion(Eigen::AngleAxisf(pitch_delta, right_axis_));
    quatf yaw_quaternion(Eigen::AngleAxisf(yaw_delta, up_axis_));
    orientation_ = pitch_quaternion * yaw_quaternion * orientation_;

    pitch_ = smooth_pitch;
    yaw_ = smooth_yaw;

    position_ += translation_smooth_factor_ * (target_position_ - position_);
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::reset_all() {
  reset_position();
  reset_orientation();

  rotation_smooth_factor_    = CGAL_CLIPPING_PLANE_ROTATION_SMOOTHNESS;
  translation_smooth_factor_ = CGAL_CLIPPING_PLANE_TRANSLATION_SMOOTHNESS;
  rotation_speed_            = CGAL_CLIPPING_PLANE_ROTATION_SPEED;
  translation_speed_         = CGAL_CLIPPING_PLANE_TRANSLATION_SPEED;

  size_ = 1.0f;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::reset_orientation() {
  orientation_ = quatf::Identity();
  pitch_ = 0.0f;
  target_pitch_ = 0.0f;
  yaw_ = 0.0f;
  target_yaw_ = 0.0f;

  up_axis_ = vec3f::UnitY();
  right_axis_ = vec3f::UnitX();
}

CGAL_INLINE_FUNCTION
void Clipping_plane::reset_position() {
  position_ = vec3f::Zero();
  target_position_ = vec3f::Zero();
}

CGAL_INLINE_FUNCTION
vec3f Clipping_plane::get_normal() const {
  return (orientation_ * vec3f::UnitZ()).normalized();
}

CGAL_INLINE_FUNCTION
mat4f Clipping_plane::get_matrix() const {
  mat4f translation = transform::translation(position_);
  mat4f rotation = transform::rotation(orientation_);

  return translation * rotation;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::rotation(const float x, const float y) {
  if (constraint_axis_ == Constraint_axis::NO_CONSTRAINT ||
      constraint_axis_ == Constraint_axis::RIGHT_AXIS) {
    target_pitch_ += y;
  }

  if (constraint_axis_ == Constraint_axis::NO_CONSTRAINT ||
      constraint_axis_ == Constraint_axis::UP_AXIS) {
    target_yaw_ += x;
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::translation(const float s) {
  vec3f normal = get_normal();
  target_position_ -= size_ * normal * s * translation_speed_;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::translation(const float x, const float y) {
  target_position_ -= size_ * right_axis_ * x * translation_speed_;
  target_position_ -= size_ * up_axis_ * y * translation_speed_;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::translation(const vec3f& direction, const float s) {
  target_position_ -= size_ * direction.normalized() * s * translation_speed_;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::switch_constraint_axis() {
  switch (constraint_axis_) {
  case Constraint_axis::NO_CONSTRAINT:
    constraint_axis_ = Constraint_axis::RIGHT_AXIS;
    break;
  case Constraint_axis::RIGHT_AXIS:
    constraint_axis_ = Constraint_axis::UP_AXIS;
    break;
  case Constraint_axis::UP_AXIS:
    constraint_axis_ = Constraint_axis::NO_CONSTRAINT;
    break;
  }
}

CGAL_INLINE_FUNCTION
bool Clipping_plane::need_update() const {
  return !utils::equal_float(target_pitch_, pitch_)
      || !utils::equal_float(target_yaw_, yaw_)
      || !utils::equal_vec3f(target_position_, position_);
}

CGAL_INLINE_FUNCTION
void Clipping_plane::set_orientation(const vec3f& normal) {
  orientation_ = quatf::FromTwoVectors(vec3f::UnitZ(), normal);
}

CGAL_INLINE_FUNCTION
void Clipping_plane::align_to_direction(const vec3f& direction) {
  vec3f normal = get_normal();
  float dot_ff = normal.dot(-direction);
  float dot_fb = normal.dot(direction);

  pitch_ = target_pitch_;
  yaw_ = target_yaw_;
  position_ = target_position_;

  if (dot_ff > dot_fb) {
    orientation_ = quatf::FromTwoVectors(normal, -direction) * orientation_;
  } else {
    orientation_ = quatf::FromTwoVectors(normal, direction) * orientation_;
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::switch_display_mode() {
  switch (display_mode_) {
  case Display_mode::OFF:
    display_mode_ = Display_mode::SOLID_HALF_TRANSPARENT_HALF;
    break;
  case Display_mode::SOLID_HALF_TRANSPARENT_HALF:
    display_mode_ = Display_mode::SOLID_HALF_WIRE_HALF;
    break;
  case Display_mode::SOLID_HALF_WIRE_HALF:
    display_mode_ = Display_mode::SOLID_HALF_ONLY;
    break;
  case Display_mode::SOLID_HALF_ONLY:
    display_mode_ = Display_mode::OFF;
    break;
  default:
    break; 
  }
}

void Clipping_plane::display_mode(Clipping_plane::Display_mode mode) {
  display_mode_ = mode; 
}

CGAL_INLINE_FUNCTION
void Clipping_plane::toggle_rendering() {
  render_clipping_plane_ = !render_clipping_plane_; 
}

CGAL_INLINE_FUNCTION
Clipping_plane::Display_mode Clipping_plane::display_mode() const {
  return display_mode_; 
}

CGAL_INLINE_FUNCTION
bool Clipping_plane::display_mode_is(Clipping_plane::Display_mode mode) const {
  return display_mode_ == mode; 
}

CGAL_INLINE_FUNCTION
bool Clipping_plane::display_mode_enabled() const {
  return display_mode_ != Clipping_plane::Display_mode::OFF; 
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_CLIPPING_PLANE_IMPL_H
