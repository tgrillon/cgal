#ifndef CGAL_GLFW_INTERNAL_ANIMATION_CONTROLLER_IMPL_H
#define CGAL_GLFW_INTERNAL_ANIMATION_CONTROLLER_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Animation_controller.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <cassert>
#include <cmath>
#include <chrono>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Animation_controller::Animation_controller() {}

CGAL_INLINE_FUNCTION
void Animation_controller::start() {
  if (key_frames_.size() <= 1) {
    return;
  }

  if (!is_running_) {
    start_time_ = std::chrono::steady_clock::now();
    is_running_ = true;
  }
}

CGAL_INLINE_FUNCTION
Animation_key_frame Animation_controller::run() {
  auto now = std::chrono::steady_clock::now();
  float elapsed_time = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count());
  current_frame_number_ = last_frame_number_ + elapsed_time;
  if (current_frame_number_ >= static_cast<float>(duration_.count())) {
    current_frame_number_ = 0.0;
    stop(0.0);
    return last_frame_data_;
  }

  last_frame_data_ = key_frame_interpolation(current_frame_number_);
  return last_frame_data_;
}

CGAL_INLINE_FUNCTION
void Animation_controller::stop(const float frame_number) {
  is_running_ = false;
  last_frame_number_ = frame_number;
}

CGAL_INLINE_FUNCTION
void Animation_controller::add_key_frame(const vec3f& position, const quatf& orientation) {
  Animation_key_frame key_frame(position, orientation);
  key_frames_.push_back(key_frame);
  if (key_frames_.size() > 1) {
    compute_timestamp();
  }
}

CGAL_INLINE_FUNCTION
void Animation_controller::set_duration(Duration_type duration) {
  duration_ = duration;
  if (key_frames_.size() > 1) {
    compute_timestamp();
  }
}

CGAL_INLINE_FUNCTION
Animation_key_frame Animation_controller::key_frame_interpolation(const float time) {
  assert(!utils::equal_float(timestamp_, 0));

  float t = time / timestamp_;

  unsigned int lower_index = std::floor(t);
  unsigned int upper_index = std::ceil(t);

  Animation_key_frame key_frame_0 = key_frames_.at(lower_index);
  Animation_key_frame key_frame_1 = key_frames_.at(upper_index);

  t -= static_cast<float>(lower_index);

  interpolated_rotation_ = key_frame_0.orientation.slerp(t, key_frame_1.orientation);

  interpolated_translation_ = utils::lerp(key_frame_0.position, key_frame_1.position, t);

  return { interpolated_translation_, interpolated_rotation_ };
}

CGAL_INLINE_FUNCTION
void Animation_controller::compute_timestamp() {
  assert(key_frames_.size() > 1);
  timestamp_ = static_cast<float>(duration_.count()) / static_cast<float>(key_frames_.size() - 1);
}

CGAL_INLINE_FUNCTION 
bool Animation_controller::is_running() const { 
  return is_running_; 
}

CGAL_INLINE_FUNCTION 
void Animation_controller::clear_buffer() { 
  key_frames_.clear(); 
}

CGAL_INLINE_FUNCTION 
quatf Animation_controller::get_rotation() const { 
  return interpolated_rotation_; 
}

CGAL_INLINE_FUNCTION 
vec3f Animation_controller::get_translation() const { 
  return interpolated_translation_; 
}

CGAL_INLINE_FUNCTION 
float Animation_controller::get_frame() const { 
  return current_frame_number_; 
}

CGAL_INLINE_FUNCTION 
size_t Animation_controller::number_of_key_frames() const { 
  return key_frames_.size(); 
} 

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_ANIMATION_CONTROLLER_IMPL_H
