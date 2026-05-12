#ifndef CGAL_GLFW_INTERNAL_CLIPPING_PLANE_IMPL_H
#define CGAL_GLFW_INTERNAL_CLIPPING_PLANE_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Clipping_plane.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <CGAL/GLFW/internal/render/Line_renderer.h>


namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
void Clipping_plane::update(const float dt) {
  if (need_update()) {
    float kr = std::min(rotation_speed_ * dt, 1.0f); 
    float pitch_delta = utils::radians(kr * (target_pitch_ - pitch_));
    float yaw_delta = utils::radians(kr * (target_yaw_ - yaw_));

    if (constraint_axis_ == Constraint_axis::FORWARD_AXIS) {
      quatf roll_quaternion(Eigen::AngleAxisf(pitch_delta - yaw_delta, -forward_axis_));
      orientation_ = roll_quaternion * orientation_;
    } else {
      quatf pitch_quaternion(Eigen::AngleAxisf(pitch_delta, right_axis_));
      quatf yaw_quaternion(Eigen::AngleAxisf(yaw_delta, up_axis_));
      orientation_ = pitch_quaternion * yaw_quaternion * orientation_;
    }

    pitch_ += kr * (target_pitch_ - pitch_);
    yaw_ += kr * (target_yaw_ - yaw_);

    float kt = std::min(translation_smooth_rate_ * dt, 1.0f);
    position_ += kt * (target_position_ - position_);
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::reset_all() {
  reset_position();
  reset_orientation();

  rotation_speed_ = CGAL_GLFW_CLIPPING_PLANE_ROTATION_SPEED;
  translation_speed_ = CGAL_GLFW_CLIPPING_PLANE_TRANSLATION_SPEED;
  rotation_smooth_rate_ = CGAL_GLFW_CLIPPING_PLANE_ROTATION_SMOOTH_RATE;
  translation_smooth_rate_ = CGAL_GLFW_CLIPPING_PLANE_TRANSLATION_SMOOTH_RATE;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::reset_orientation() {
  orientation_ = quatf::Identity();
  pitch_ = 0.0f;
  target_pitch_ = 0.0f;
  yaw_ = 0.0f;
  target_yaw_ = 0.0f;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::reset_position() {
  position_ = vec3f::Zero();
  target_position_ = vec3f::Zero();
}

CGAL_INLINE_FUNCTION
vec3f Clipping_plane::normal() const {
  return (orientation_ * vec3f::UnitZ()).normalized();
}

CGAL_INLINE_FUNCTION
mat4f Clipping_plane::model_matrix() const {
  mat4f translation = transform::translation(position_);
  mat4f rotation = transform::rotation(orientation_);

  return translation * rotation;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::rotation(const float x, const float y) {
  if (constraint_axis_ == Constraint_axis::NO_CONSTRAINT ||
      constraint_axis_ == Constraint_axis::RIGHT_AXIS    ||
      constraint_axis_ == Constraint_axis::FORWARD_AXIS) {
    target_pitch_ += y;
  }

  if (constraint_axis_ == Constraint_axis::NO_CONSTRAINT ||
      constraint_axis_ == Constraint_axis::UP_AXIS       ||
      constraint_axis_ == Constraint_axis::FORWARD_AXIS) {
    target_yaw_ += x;
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::translation(const float s) {
  target_position_ -= translation_scale_ * normal() * s * translation_speed_;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::translation(const float x, const float y) {
  target_position_ -= translation_scale_ * right_axis_ * x * translation_speed_;
  target_position_ -= translation_scale_ * up_axis_ * y * translation_speed_;
}

CGAL_INLINE_FUNCTION
void Clipping_plane::translation(const vec3f& direction, const float s) {
  target_position_ -= translation_scale_ * direction.normalized() * s * translation_speed_;
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
    constraint_axis_ = Constraint_axis::FORWARD_AXIS;
    break;
  case Constraint_axis::FORWARD_AXIS:
    constraint_axis_ = Constraint_axis::NO_CONSTRAINT;
    break; 
  }
}

CGAL_INLINE_FUNCTION
std::string Clipping_plane::constraint_axis_str() const {
  switch(constraint_axis_) {
    case Constraint_axis::NO_CONSTRAINT:  return "None"; 
    case Constraint_axis::RIGHT_AXIS:     return "Right"; 
    case Constraint_axis::UP_AXIS:        return "Up"; 
    case Constraint_axis::FORWARD_AXIS:   return "Forward"; 
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::increase_rotation_speed(float dt) { 
  rotation_speed_ = std::min(rotation_speed_ + 100.f * dt, 360.f); 
}

CGAL_INLINE_FUNCTION
void Clipping_plane::decrease_rotation_speed(float dt) { 
  rotation_speed_ = std::max(rotation_speed_ - 100.f * dt, 60.f); 
}

CGAL_INLINE_FUNCTION
void Clipping_plane::increase_translation_speed(float dt) { 
  translation_speed_ = std::min(translation_speed_ + dt, 20.f); 
}

CGAL_INLINE_FUNCTION
void Clipping_plane::decrease_translation_speed(float dt) { 
  translation_speed_ = std::max(translation_speed_ - dt, 0.05f); 
}

CGAL_INLINE_FUNCTION
void Clipping_plane::align_to_direction(const vec3f& direction) {
  vec3f n = normal();
  float dot_ff = n.dot(-direction);
  float dot_fb = n.dot(direction);

  pitch_ = target_pitch_;
  yaw_ = target_yaw_;
  position_ = target_position_;

  if (dot_ff > dot_fb) {
    orientation_ = quatf::FromTwoVectors(n, -direction) * orientation_;
  } else {
    orientation_ = quatf::FromTwoVectors(n, direction) * orientation_;
  }
}

CGAL_INLINE_FUNCTION
void Clipping_plane::switch_clipping_mode() {
  switch (clipping_mode_) {
  case Clipping_mode::OFF:
    clipping_mode_ = Clipping_mode::SOLID_HALF_TRANSPARENT_HALF;
    break;
  case Clipping_mode::SOLID_HALF_TRANSPARENT_HALF:
    clipping_mode_ = Clipping_mode::SOLID_HALF_WIRE_HALF;
    break;
  case Clipping_mode::SOLID_HALF_WIRE_HALF:
    clipping_mode_ = Clipping_mode::SOLID_HALF_ONLY;
    break;
  case Clipping_mode::SOLID_HALF_ONLY:
    clipping_mode_ = Clipping_mode::OFF;
    break;
  }
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_CLIPPING_PLANE_IMPL_H
