#ifndef CGAL_GLFW_INTERNAL_CAMERA_IMPL_H
#define CGAL_GLFW_INTERNAL_CAMERA_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Camera.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>

#include <Eigen/Geometry>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
void Camera::update(const float dt) {
  if (need_update()) {
    if (type_ == Camera_type::FREE_FLY) {
      if (target_pitch_ > 90.f) {
        target_pitch_ = 90.f;
      } else if (target_pitch_ < -90.f) {
        target_pitch_ = -90.f;
      }
    }

    float smooth_pitch = pitch_ + rotation_smooth_factor_ * (target_pitch_ - pitch_);
    float smooth_yaw = yaw_ + rotation_smooth_factor_ * (target_yaw_ - yaw_);

    float pitch_delta = utils::radians((smooth_pitch - pitch_) * rotation_speed_) * dt;
    float yaw_delta = utils::radians((smooth_yaw - yaw_) * rotation_speed_) * dt;

    if (is_orbiter()) {
      if (constraint_axis_ == Constraint_axis::FORWARD_AXIS) {
        quatf roll_quaternion(Eigen::AngleAxisf(pitch_delta - yaw_delta, -vec3f::UnitZ()));
        orientation_ = roll_quaternion * orientation_;
      } else {
        quatf pitch_quaternion(Eigen::AngleAxisf(pitch_delta, vec3f::UnitX()));
        quatf yaw_quaternion(Eigen::AngleAxisf(yaw_delta, vec3f::UnitY()));
        orientation_ = pitch_quaternion * yaw_quaternion * orientation_;
      }
    } else { // free fly
      quatf pitch_quaternion(Eigen::AngleAxisf(pitch_delta, get_right()));
      quatf yaw_quaternion(Eigen::AngleAxisf(yaw_delta, vec3f::UnitY())); // lock roll rotation
      orientation_ = orientation_ * pitch_quaternion * yaw_quaternion;
    }
    pitch_ = smooth_pitch;
    yaw_ = smooth_yaw;

    position_ += translation_smooth_factor_ * (target_position_ - position_);

    size_ += zoom_smooth_factor_ * (target_size_ - size_);
  }
}

CGAL_INLINE_FUNCTION
void Camera::lookat(const vec3f& center, const float size) {
  center_ = center; // camera focus
  size_ = size;
  default_size_ = size; // allowing reset
  radius_ = size;

  compute_target_size();
}

CGAL_INLINE_FUNCTION
void Camera::lookat(const vec3f& pmin, const vec3f& pmax) {
  lookat(utils::center(pmin, pmax), utils::distance(pmin, pmax));
}

CGAL_INLINE_FUNCTION
void Camera::move(const float z) {
  if (is_orbiter()) {
    target_size_ -= target_size_ * z;
    if (target_size_ < 0.001f)
      target_size_ = 0.001f;
  } else { // free fly
    vec3f forward = get_forward();
    target_position_ += forward * size_ * z * translation_speed_;
  }
}

CGAL_INLINE_FUNCTION
void Camera::rotation(const float x, const float y) {
  if (constraint_axis_ == Constraint_axis::NO_CONSTRAINT ||
      constraint_axis_ == Constraint_axis::RIGHT_AXIS    ||
      constraint_axis_ == Constraint_axis::FORWARD_AXIS) {
    target_pitch_ += y + y / rotation_smooth_factor_ * .1;
  }

  if (constraint_axis_ == Constraint_axis::NO_CONSTRAINT ||
      constraint_axis_ == Constraint_axis::UP_AXIS       ||
      constraint_axis_ == Constraint_axis::FORWARD_AXIS) {
    target_yaw_ += x + x / rotation_smooth_factor_ * .1;
  }
}

CGAL_INLINE_FUNCTION
void Camera::translation(const float x, const float y) {
  float x_speed = x * translation_speed_ + x / translation_smooth_factor_ * .01;
  float y_speed = y * translation_speed_ + y / translation_smooth_factor_ * .01;

  if (is_orbiter()) {
    target_position_.x() += size_ * x_speed;
    target_position_.y() += size_ * y_speed;
  } else { // free fly
    vec3f right = get_right();
    vec3f up = get_up();
    target_position_ -= right * size_ * x_speed;
    target_position_ -= up * size_ * y_speed;
  }
}

CGAL_INLINE_FUNCTION
void Camera::move_up(const float dt) {
  if (is_orbiter()) {
    target_position_.y() += size_ * dt * translation_speed_;
  } else { // free fly
    vec3f up = get_up();
    target_position_ += up * size_ * dt * translation_speed_;
  }
}

CGAL_INLINE_FUNCTION
void Camera::move_down(const float dt) {
  move_up(-dt);
}

CGAL_INLINE_FUNCTION
void Camera::move_right(const float dt) {
  if (is_orbiter()) {
    target_position_.x() += size_ * dt * translation_speed_;
  } else { // free fly
    vec3f right = get_right();
    target_position_ += right * size_ * dt * translation_speed_;
  }
}

CGAL_INLINE_FUNCTION
void Camera::move_left(const float dt) {
  move_right(-dt);
}

CGAL_INLINE_FUNCTION
mat4f Camera::view() const {
  mat4f rotation = transform::rotation(orientation_);

  if (is_orbiter()) {
    mat4f translation = transform::translation(-position_.x(), -position_.y(), -size_);
    mat4f model = transform::translation(-center_.x(), -center_.y(), -center_.z());
    return translation * rotation * model;
  } else { // free fly
    mat4f translation = transform::translation(-position_.x(), -position_.y(), -position_.z() - size_); // translate camera to (0,0,0)
    return rotation * translation;
  }
}

CGAL_INLINE_FUNCTION
mat4f Camera::projection(const float width, const float height) {
  width_ = width;
  height_ = height;

  return projection();
}

CGAL_INLINE_FUNCTION
mat4f Camera::projection() const {
  if (mode_ == Camera_mode::ORTHOGRAPHIC) {
    float aspect = width_ / height_;
    float half_width = size_ * aspect * 0.5f;
    float half_height = size_ * 0.5f;
    return utils::ortho(-half_width, half_width, -half_height, half_height, znear(), zfar());
  }

  return utils::perspective(utils::radians(fov_), width_ / height_, znear(), zfar());
}

CGAL_INLINE_FUNCTION
float Camera::znear() const {
  float d;
  if (is_orbiter()) {
    d = utils::distance(center_, vec3f(position_.x(), position_.y(), size_));
  } else {
    d = utils::distance(center_, vec3f(position_.x(), position_.y(), position_.z() + size_));
  }
  return std::max(0.1f, d - 2 * radius_);
}

CGAL_INLINE_FUNCTION
float Camera::zfar() const {
  float d;
  if (is_orbiter()) {
    d = utils::distance(center_, vec3f(position_.x(), position_.y(), size_));
  } else { // free fly
    d = utils::distance(center_, vec3f(position_.x(), position_.y(), position_.z() + size_));
  }

  return std::max(100.f, d + 2 * radius_);
}

CGAL_INLINE_FUNCTION
mat4f Camera::viewport() const {
  return transform::viewport(width_, height_);
}

CGAL_INLINE_FUNCTION
void Camera::set_position(const vec3f& position) {
  position_ = position;
  target_position_ = position;
  size_ = position.z();
  target_size_ = position.z();
}

CGAL_INLINE_FUNCTION
vec3f Camera::get_position() const {
  if (is_orbiter()) {
    return vec3f(position_.x(), position_.y(), size_);
  } else { // free fly
    return vec3f(position_.x(), position_.y(), position_.z() + size_);
  }
}

CGAL_INLINE_FUNCTION
vec3f Camera::get_forward() const {
  return (orientation_.inverse() * -vec3f::UnitZ()).normalized();
}

CGAL_INLINE_FUNCTION
vec3f Camera::get_right() const {
  return (orientation_.inverse() * vec3f::UnitX()).normalized();
}

CGAL_INLINE_FUNCTION
vec3f Camera::get_up() const {
  return (orientation_.inverse() * vec3f::UnitY()).normalized();
}

CGAL_INLINE_FUNCTION
void Camera::reset_all() {
  reset_position();
  reset_orientation();
  fov_                       = CGAL_CAMERA_FOV;
  zoom_smooth_factor_        = CGAL_CAMERA_ZOOM_SMOOTHNESS;
  rotation_smooth_factor_    = CGAL_CAMERA_ROTATION_SMOOTHNESS;
  translation_smooth_factor_ = CGAL_CAMERA_TRANSLATION_SMOOTHNESS;
  translation_speed_         = CGAL_CAMERA_TRANSLATION_SPEED;
  rotation_speed_            = CGAL_CAMERA_ROTATION_SPEED;
  reset_size();
}

CGAL_INLINE_FUNCTION
void Camera::reset_size() {
  size_ = default_size_;
  compute_target_size();
}

CGAL_INLINE_FUNCTION
void Camera::reset_position() {
  position_ = default_position_;
  target_position_ = default_position_;
}

CGAL_INLINE_FUNCTION
void Camera::reset_orientation() {
  pitch_ = 0.f;
  yaw_ = 0;
  target_pitch_ = 0.f;
  target_yaw_ = 0;
  orientation_ = default_orientation_;
}

CGAL_INLINE_FUNCTION
void Camera::toggle_type() {
  type_ = (type_ == Camera_type::ORBITER) ? Camera_type::FREE_FLY : Camera_type::ORBITER;
  reset_all();
}

CGAL_INLINE_FUNCTION
void Camera::switch_constraint_axis() {
  if (!is_orbiter() && constraint_axis_ == Constraint_axis::UP_AXIS) {
    constraint_axis_ = Constraint_axis::NO_CONSTRAINT;
    return;
  }

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
void Camera::set_constraint_axis(Constraint_axis axis) {
  constraint_axis_ = axis;
}

CGAL_INLINE_FUNCTION
void Camera::increase_fov(const float d) {
  fov_ += d * 2.f;
  if (fov_ > 90.f) fov_ = 90.;
  if (fov_ < 45.f) fov_ = 45.f;

  compute_target_size();
}

CGAL_INLINE_FUNCTION
void Camera::disable_smoothness() {
  zoom_smooth_factor_ = 1.0;
  translation_smooth_factor_ = 1.0;
  rotation_smooth_factor_ = 1.0;
}

CGAL_INLINE_FUNCTION
void Camera::increase_zoom_smoothness(const float s) {
  zoom_smooth_factor_ += s * 0.01;
  if (zoom_smooth_factor_ > 1.f) zoom_smooth_factor_ = 1.;
  if (zoom_smooth_factor_ < 0.01f) zoom_smooth_factor_ = .01;
}

using Nearest_axis_result = std::pair<vec3f, vec3f>;

CGAL_INLINE_FUNCTION
Nearest_axis_result nearest_axis(const vec3f& forward, const vec3f& up) {
  std::vector<vec3f> axis = {
    vec3f::UnitX(), -vec3f::UnitX(),
    vec3f::UnitY(), -vec3f::UnitY(),
    vec3f::UnitZ(), -vec3f::UnitZ()
  };

  float max_forward_dot = -1.0f;
  float max_up_dot = -1.0f;
  vec3f nearest_forward_axis = vec3f::Zero();
  vec3f nearest_up_axis = vec3f::Zero();

  for (const auto& a : axis) {
    float dot_fw = -forward.dot(a);
    float dot_up = up.dot(a);
    if (dot_fw > max_forward_dot) {
      max_forward_dot = dot_fw;
      nearest_forward_axis = a;
    }

    if (dot_up > max_up_dot) {
      max_up_dot = dot_up;
      nearest_up_axis = a;
    }
  }

  return { nearest_forward_axis, nearest_up_axis };
}

CGAL_INLINE_FUNCTION
quatf compute_rotation(const vec3f& nearest_forward_axis, const vec3f& nearest_up_axis) {
  quatf rotation{};
  if (nearest_forward_axis == vec3f::UnitX()) {
    rotation = quatf(Eigen::AngleAxisf(-M_PI_2, vec3f::UnitY()));
    if (nearest_up_axis == -vec3f::UnitY()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI, nearest_forward_axis));
    } else if (nearest_up_axis == vec3f::UnitZ()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitZ()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI_2, nearest_forward_axis));
    }
  } else if (nearest_forward_axis == -vec3f::UnitX()) {
    rotation = quatf(Eigen::AngleAxisf(M_PI_2, vec3f::UnitY()));
    if (nearest_up_axis == -vec3f::UnitY()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI, nearest_forward_axis));
    } else if (nearest_up_axis == vec3f::UnitZ()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitZ()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI_2, nearest_forward_axis));
    }
  } else if (nearest_forward_axis == vec3f::UnitY()) {
    rotation = quatf(Eigen::AngleAxisf(M_PI_2, vec3f::UnitX()));
    if (nearest_up_axis == vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == vec3f::UnitZ()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI, nearest_forward_axis));
    }
  } else if (nearest_forward_axis == -vec3f::UnitY()) {
    rotation = quatf(Eigen::AngleAxisf(-M_PI_2, vec3f::UnitX()));
    if (nearest_up_axis == vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitZ()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI, nearest_forward_axis));
    }
  } else if (nearest_forward_axis == vec3f::UnitZ()) {
    rotation = quatf(Eigen::AngleAxisf(0.f, vec3f::UnitY()));
    if (nearest_up_axis == -vec3f::UnitY()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI, nearest_forward_axis));
    } else if (nearest_up_axis == vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI_2, nearest_forward_axis));
    }
  } else if (nearest_forward_axis == -vec3f::UnitZ()) {
    rotation = quatf(Eigen::AngleAxisf(M_PI, vec3f::UnitY()));
    if (nearest_up_axis == -vec3f::UnitY()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI, nearest_forward_axis));
    } else if (nearest_up_axis == vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(-M_PI_2, nearest_forward_axis));
    } else if (nearest_up_axis == -vec3f::UnitX()) {
      rotation *= quatf(Eigen::AngleAxisf(M_PI_2, nearest_forward_axis));
    }
  }

  return rotation;
}

CGAL_INLINE_FUNCTION
void Camera::align_to_nearest_axis() {
  vec3f forward_direction = get_forward();
  vec3f up_direction = get_up();

  auto [nearest_forward_axis, nearest_up_axis] = nearest_axis(forward_direction, up_direction);

  pitch_ = target_pitch_;
  yaw_ = target_yaw_;
  position_ = target_position_;

  orientation_ = compute_rotation(nearest_forward_axis, nearest_up_axis);
}

CGAL_INLINE_FUNCTION
void Camera::align_to_plane(const vec3f& normal) {
  vec3f forward = get_forward();
  float dot_ff = forward.dot(-normal);
  float dot_fb = forward.dot(normal);

  pitch_ = target_pitch_;
  yaw_ = target_yaw_;
  position_ = target_position_;

  if (dot_ff > dot_fb) {
    orientation_ *= quatf::FromTwoVectors(forward, -normal).inverse();
  } else {
    orientation_ *= quatf::FromTwoVectors(forward,  normal).inverse();
  }
}

CGAL_INLINE_FUNCTION
void Camera::compute_target_size() {
  float tan_half_fov = std::tan(utils::radians(fov_) * .5f);
  target_size_ = radius_ / tan_half_fov * .6f;
}

CGAL_INLINE_FUNCTION
bool Camera::need_update() const {
  return !utils::equal_float(target_pitch_, pitch_)
      || !utils::equal_float(target_yaw_, yaw_)
      || !utils::equal_float(target_size_, size_)
      || !utils::equal_vec3f(target_position_, position_);
}

CGAL_INLINE_FUNCTION
void Camera::set_orientation(const vec3f& forward, float up_angle) {
  orientation_ = quatf::FromTwoVectors(-vec3f::UnitZ(), forward.normalized()).inverse();
  orientation_ *= quatf(Eigen::AngleAxisf(utils::radians(up_angle), get_forward()));
}

CGAL_INLINE_FUNCTION
std::string Camera::get_constraint_axis_str() const {
  if (constraint_axis_ == Constraint_axis::UP_AXIS) return "Up";
  if (constraint_axis_ == Constraint_axis::RIGHT_AXIS) return "Right";
  if (constraint_axis_ == Constraint_axis::FORWARD_AXIS) return "Forward";
  return "None";
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_CAMERA_IMPL_H
