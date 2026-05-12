#ifndef CGAL_GLFW_INTERNAL_CAMERA_H
#define CGAL_GLFW_INTERNAL_CAMERA_H

#include <CGAL/config.h>

#include <vector>

#include <CGAL/GLFW/internal/input/input_codes.h>
#include <CGAL/GLFW/internal/math/math_types.h>

#include <CGAL/GLFW/bv_settings.h>

namespace CGAL {
namespace GLFW {
namespace internal {

class Camera
{
public:
  enum class Constraint_axis { NO_CONSTRAINT, RIGHT_AXIS, UP_AXIS, FORWARD_AXIS };
  enum class Camera_mode { PERSPECTIVE, ORTHOGRAPHIC };
  enum class Camera_type { ORBITER, FREE_FLY };

public:
  Camera()=default;

  void update(const float dt);

  void lookat(const vec3f& center, const float size);
  void lookat(const vec3f& pmin, const vec3f& pmax);

  void move(const float z);
  void rotation(const float x, const float y);
  void translation(const float x, const float y);

  void move_up(const float dt);
  void move_down(const float dt);
  void move_right(const float dt);
  void move_left(const float dt);

  mat4f view() const;

  mat4f projection() const;
  mat4f projection(float width, float height) const;

  float znear() const;
  float zfar() const;

  mat4f viewport() const;

  void set_position(const vec3f& position);
  vec3f get_position() const;

  vec3f get_forward() const;
  vec3f get_right() const;
  vec3f get_up() const;

  void reset_all();
  void reset_position();
  void reset_orientation();
  void reset_size();

  void toggle_type();

  void switch_constraint_axis();
  void set_constraint_axis(Constraint_axis axis);

  void increase_fov(float d);
  void disable_smoothness();
  void increase_zoom_smoothness(const float dt);

  void align_to_plane(const vec3f& normal);
  void align_to_nearest_axis();

  std::string get_constraint_axis_str() const;

  bool need_update() const;

  inline float get_translation_speed() const { return translation_speed_; }
  inline float get_rotation_speed() const { return rotation_speed_; }

  inline float get_size() const { return size_; }
  inline float get_radius() const { return radius_; }
  inline vec3f get_center() const { return center_; }

  inline float get_fov() const { return fov_; }

  inline quatf get_orientation() const { return orientation_; }

  inline void set_default_size(float size) { default_size_ = size; }
  inline void set_default_position(const vec3f& position) { default_position_ = position; set_default_size(position.z()); }
  inline void set_default_orientation(const quatf& orientation) { default_orientation_ = orientation; }

  inline void set_size(float size) { size_ = size; target_size_ = size; }
  inline void set_radius(float radius) { radius_ = radius; }
  inline void set_center(const vec3f& center) { center_ = center; }

  inline void set_orthographic() { mode_ = Camera_mode::ORTHOGRAPHIC; }
  inline void set_mode(Camera_mode mode) { mode_ = mode; }
  inline void set_orientation(const quatf& orientation) { orientation_ = orientation; }

  void set_orientation(const vec3f& forward, float up_angle);

  inline void toggle_mode() { mode_ = (mode_ == Camera_mode::ORTHOGRAPHIC) ? Camera_mode::PERSPECTIVE : Camera_mode::ORTHOGRAPHIC; }
  inline bool is_orthographic() const { return mode_ == Camera_mode::ORTHOGRAPHIC; }
  inline bool is_orbiter() const { return type_ == Camera_type::ORBITER; }

  inline void increase_rotation_speed(const float dt) { rotation_speed_ = std::min(rotation_speed_ + 100.f * dt, 360.f); }
  inline void decrease_rotation_speed(const float dt) { rotation_speed_ = std::max(rotation_speed_ - 100.f * dt, 60.f); }

  inline void increase_translation_speed(const float dt) { translation_speed_ = std::min(translation_speed_ + dt, 20.f); }
  inline void decrease_translation_speed(const float dt) { translation_speed_ = std::max(translation_speed_ - dt, 0.5f); }

  inline void increase_rotation_smoothness(const float dt) { rotation_smooth_factor_ = std::max(rotation_smooth_factor_ - dt, 0.01f); }
  inline void decrease_rotation_smoothness(const float dt) { rotation_smooth_factor_ = std::min(rotation_smooth_factor_ + dt, 1.f); }

  inline void increase_translation_smoothness(const float dt) { translation_smooth_factor_ = std::max(translation_smooth_factor_ - dt, 0.01f); }
  inline void decrease_translation_smoothness(const float dt) { translation_smooth_factor_ = std::min(translation_smooth_factor_ + dt, 1.f); }

private:
  void compute_target_size();

private:
  quatf orientation_         { quatf::Identity() };
  quatf default_orientation_ { quatf::Identity() };

  vec3f center_ { vec3f::Zero() };

  vec3f position_         { vec3f::Zero() };
  vec3f target_position_  { vec3f::Zero() };
  vec3f default_position_ { vec3f::Zero() };

  float size_         { CGAL_GLFW_CAMERA_RADIUS };
  float default_size_ { CGAL_GLFW_CAMERA_RADIUS };
  float target_size_  { CGAL_GLFW_CAMERA_RADIUS };
  float radius_       { CGAL_GLFW_CAMERA_RADIUS };

  mutable float width_  { 1.0f };
  mutable float height_ { 1.0f };
  float fov_    { CGAL_GLFW_CAMERA_FOV };

  float rotation_speed_    { CGAL_GLFW_CAMERA_ROTATION_SPEED };
  float translation_speed_ { CGAL_GLFW_CAMERA_TRANSLATION_SPEED };

  Camera_type type_ { Camera_type::ORBITER };
  Camera_mode mode_ { Camera_mode::PERSPECTIVE };

  Constraint_axis constraint_axis_ { Constraint_axis::NO_CONSTRAINT };

  float pitch_        { 0.0f };
  float target_pitch_ { 0.0f };
  float yaw_          { 0.0f };
  float target_yaw_   { 0.0f };

  float zoom_smooth_factor_        { CGAL_GLFW_CAMERA_ZOOM_SMOOTHNESS };
  float rotation_smooth_factor_    { CGAL_GLFW_CAMERA_ROTATION_SMOOTHNESS };
  float translation_smooth_factor_ { CGAL_GLFW_CAMERA_TRANSLATION_SMOOTHNESS };
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Camera_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_CAMERA_H
