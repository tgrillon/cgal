#ifndef CGAL_GLFW_INTERNAL_ANIMATION_CONTROLLER_H
#define CGAL_GLFW_INTERNAL_ANIMATION_CONTROLLER_H

#include <CGAL/config.h>

#include <vector>
#include <chrono>

#include "utils.h"

namespace CGAL {
namespace GLFW {
namespace internal {

struct Animation_key_frame
{
  vec3f position;
  quatf orientation;

  Animation_key_frame(const vec3f& position, const quatf& orientation) :
    position(position),
    orientation(orientation)
  {
  }

  Animation_key_frame() :
    position(vec3f::Identity()),
    orientation(quatf::Identity())
  {
  }
};

class Animation_controller
{
public:
  using Duration_type = std::chrono::milliseconds;
  using Time_point = std::chrono::_V2::steady_clock::time_point;
  using Key_frame_buffer = std::vector<Animation_key_frame>;

public:
  void start();
  Animation_key_frame run();
  void stop(const float frame_number);

  void add_key_frame(const vec3f& position, const quatf& orientation);

  void set_duration(Duration_type duration);

  inline bool is_running() const { return is_running_; }

  inline void clear_buffer() { key_frames_.clear(); }

  inline quatf get_rotation() const { return interpolated_rotation_; }

  inline vec3f get_translation() const { return interpolated_translation_; }

  inline float get_frame() const { return current_frame_number_; }

  inline size_t number_of_key_frames() const { return key_frames_.size(); }

private:
  Animation_key_frame key_frame_interpolation(const float time);

  void compute_timestamp();

private:
  quatf interpolated_rotation_ { quatf::Identity() };
  vec3f interpolated_translation_ { vec3f::Zero() };

  Duration_type duration_ { std::chrono::milliseconds(5000) };

  Key_frame_buffer key_frames_ {};

  Time_point start_time_ {};

  Animation_key_frame last_frame_data_ {};

  float last_frame_number_    { 0.0f };
  float current_frame_number_ { 0.0f };
  float timestamp_            { 0.0f };

  bool is_running_ { false };
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Animation_controller_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_ANIMATION_CONTROLLER_H
