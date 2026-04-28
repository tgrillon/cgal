#ifndef CGAL_GLFW_INTERNAL_CLIPPING_PLANE_H
#define CGAL_GLFW_INTERNAL_CLIPPING_PLANE_H

#include <CGAL/config.h>

#include "Line_renderer.h"
#include "utils.h"

#include "../bv_settings.h"

namespace CGAL {
namespace GLFW {
namespace internal {

class Clipping_plane : private Line_renderer
{
public:
  enum class Display_mode {
    OFF = 0,
    SOLID_HALF_TRANSPARENT_HALF,
    SOLID_HALF_WIRE_HALF,
    SOLID_HALF_ONLY,
  };
public:
  enum class Constraint_axis { NO_CONSTRAINT, RIGHT_AXIS, UP_AXIS };

public:
  Clipping_plane(float size);

  void render();
  
  void on_update(const float delta_time);

  void reset_all();
  void reset_position();
  void reset_orientation();

  mat4f get_matrix() const;

  void rotation(const float x, const float y);

  void translation(const float s);
  void translation(const float x, const float y);
  void translation(const vec3f& direction, const float s);

  void switch_constraint_axis();

  vec3f get_normal() const;

  bool need_update() const;

  inline float get_transparency() const { return transparency_; }
  inline float get_rotation_speed() const { return rotation_speed_; }
  inline float get_translation_speed() const { return translation_speed_; }
  inline std::string get_constraint_axis_str() const { return constraint_axis_ == Constraint_axis::NO_CONSTRAINT ? "None" : (constraint_axis_ == Constraint_axis::RIGHT_AXIS ? "Right" : "Up"); }

  void set_orientation(const vec3f& normal);

  inline void set_size(const float size) { size_ = size; }
  inline void set_up_axis(const vec3f& up_axis) { up_axis_ = up_axis; }
  inline void set_right_axis(const vec3f& right_axis) { right_axis_ = right_axis; }

  inline void increase_rotation_speed(float delta_time) { rotation_speed_ = std::min(rotation_speed_ + 100.f * delta_time, 360.f); }
  inline void decrease_rotation_speed(float delta_time) { rotation_speed_ = std::max(rotation_speed_ - 100.f * delta_time, 60.f); }

  inline void increase_translation_speed(float delta_time) { translation_speed_ = std::min(translation_speed_ + delta_time, 20.f); }
  inline void decrease_translation_speed(float delta_time) { translation_speed_ = std::max(translation_speed_ - delta_time, 0.05f); }

  inline void increase_rotation_smoothness(float delta_time) { rotation_smooth_factor_ = std::max(rotation_smooth_factor_ - delta_time, 0.01f); }
  inline void decrease_rotation_smoothness(float delta_time) { rotation_smooth_factor_ = std::min(rotation_smooth_factor_ + delta_time, 1.f); }

  inline void increase_translation_smoothness(float delta_time) { translation_smooth_factor_ = std::max(translation_smooth_factor_ - delta_time, 0.01f); }
  inline void decrease_translation_smoothness(float delta_time) { translation_smooth_factor_ = std::min(translation_smooth_factor_ + delta_time, 1.f); }

  void align_to_direction(const vec3f& direction);

  void switch_display_mode(); 
  void display_mode(Display_mode mode); 
  Display_mode display_mode() const; 
  bool display_mode_is(Display_mode mode) const; 
  bool display_mode_enabled() const; 

  void toggle_rendering(); 

private:
  quatf orientation_ { quatf::Identity() };

  vec3f position_        { vec3f::Zero() };
  vec3f target_position_ { vec3f::Zero() };

  vec3f up_axis_    { vec3f::UnitY() };
  vec3f right_axis_ { vec3f::UnitX() };

  float pitch_        { 0.0f };
  float target_pitch_ { 0.0f };
  float yaw_          { 0.0f };
  float target_yaw_   { 0.0f };

  float rotation_speed_    { CGAL_CLIPPING_PLANE_ROTATION_SPEED };
  float translation_speed_ { CGAL_CLIPPING_PLANE_TRANSLATION_SPEED };

  float rotation_smooth_factor_    { CGAL_CLIPPING_PLANE_ROTATION_SMOOTHNESS };
  float translation_smooth_factor_ { CGAL_CLIPPING_PLANE_TRANSLATION_SMOOTHNESS };

  float size_ { 1.0f };

  float transparency_ { CGAL_CLIPPING_PLANE_RENDERING_TRANSPARENCY };

  Constraint_axis constraint_axis_ { Constraint_axis::NO_CONSTRAINT };

  Display_mode display_mode_{ Display_mode::OFF }; 

  bool render_clipping_plane_{ true }; 
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Clipping_plane_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_CLIPPING_PLANE_H
