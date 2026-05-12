#ifndef CGAL_GLFW_INTERNAL_CLIPPING_PLANE_H
#define CGAL_GLFW_INTERNAL_CLIPPING_PLANE_H

#include <CGAL/GLFW/internal/render/Line_renderer.h>
#include <CGAL/GLFW/internal/math/math_utils.h>

#include <CGAL/GLFW/bv_settings.h>

namespace CGAL {
namespace GLFW {
namespace internal {

class Clipping_plane
{
public:
  enum class Clipping_mode {
    OFF = 0,
    SOLID_HALF_TRANSPARENT_HALF,
    SOLID_HALF_WIRE_HALF,
    SOLID_HALF_ONLY,
  };

  enum class Constraint_axis { 
    NO_CONSTRAINT, 
    RIGHT_AXIS, 
    UP_AXIS,
    FORWARD_AXIS,
  };

public:
  Clipping_plane() = default;

  Clipping_plane(const Clipping_plane&) = delete;
  Clipping_plane& operator=(const Clipping_plane&) = delete;
  
  Clipping_plane(Clipping_plane&& other) noexcept = default;
  Clipping_plane& operator=(Clipping_plane&& other) noexcept = default;
  
  ~Clipping_plane() = default; 
  
  void update(const float dt);

  mat4f model_matrix() const;
  vec3f normal() const;

  bool need_update() const {
    return !utils::equal_float(target_pitch_, pitch_)
        || !utils::equal_float(target_yaw_, yaw_)
        || !utils::equal_vec3f(target_position_, position_);
  }

  float transparency() const { return transparency_; }
  void size(const float size) { translation_scale_ = size; }
  
  // Orientation / position
  
  void up_axis(const vec3f& up_axis) { up_axis_ = up_axis; }
  void right_axis(const vec3f& right_axis) { right_axis_ = right_axis; }
  void forward_axis(const vec3f& forward_axis) { forward_axis_ = forward_axis; }
  
  void align_to_direction(const vec3f& direction);
  
  void orientation(const vec3f& normal) { orientation_ = quatf::FromTwoVectors(vec3f::UnitZ(), normal); }

  void rotation(const float x, const float y);

  void translation(const float s);
  void translation(const float x, const float y);
  void translation(const vec3f& direction, const float s);

  void switch_constraint_axis();
  
  std::string constraint_axis_str() const;

  void reset_all();
  void reset_position();
  void reset_orientation();

  // Translation / rotation speed 

  void increase_rotation_speed(float dt);
  void decrease_rotation_speed(float dt);
  void increase_translation_speed(float dt);
  void decrease_translation_speed(float dt);
  
  float rotation_speed() const { return rotation_speed_; }
  float translation_speed() const { return translation_speed_; }

  // Clipping mode

  void switch_clipping_mode(); 
  void clipping_mode(Clipping_mode mode) { clipping_mode_ = mode; }
  Clipping_mode clipping_mode() const { return clipping_mode_; }
  bool clipping_mode_is(Clipping_mode mode) const { return clipping_mode_ == mode; }
  bool clipping_enabled() const { return clipping_mode_ != Clipping_plane::Clipping_mode::OFF; } 

private:
  quatf orientation_ { quatf::Identity() };

  vec3f position_        { vec3f::Zero() };
  vec3f target_position_ { vec3f::Zero() };

  vec3f forward_axis_ { vec3f::UnitZ() };
  vec3f up_axis_    { vec3f::UnitY() };
  vec3f right_axis_ { vec3f::UnitX() };

  float pitch_        { 0.0f };
  float target_pitch_ { 0.0f };
  float yaw_          { 0.0f };
  float target_yaw_   { 0.0f };

  float rotation_speed_    { CGAL_GLFW_CLIPPING_PLANE_ROTATION_SPEED };
  float translation_speed_ { CGAL_GLFW_CLIPPING_PLANE_TRANSLATION_SPEED };

  float rotation_smooth_rate_    { CGAL_GLFW_CLIPPING_PLANE_ROTATION_SMOOTH_RATE };
  float translation_smooth_rate_ { CGAL_GLFW_CLIPPING_PLANE_TRANSLATION_SMOOTH_RATE };

  float translation_scale_ { 1.0f };

  float transparency_ { CGAL_GLFW_CLIPPING_PLANE_RENDERING_TRANSPARENCY };

  Constraint_axis constraint_axis_ { Constraint_axis::NO_CONSTRAINT };

  Clipping_mode clipping_mode_{ Clipping_mode::OFF }; 

private: 
  static constexpr int SUBDIVISIONS = 30;
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Clipping_plane_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_CLIPPING_PLANE_H
