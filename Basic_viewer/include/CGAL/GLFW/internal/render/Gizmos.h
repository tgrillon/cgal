#ifndef CGAL_GLFW_INTERNAL_GIZMOS_H
#define CGAL_GLFW_INTERNAL_GIZMOS_H

#include <CGAL/GLFW/internal/math/math_types.h>
#include <CGAL/GLFW/internal/render/Line_renderer.h>
#include <CGAL/GLFW/internal/render/Shader_program.h>

namespace CGAL {
namespace GLFW {
namespace internal {

class Gizmos {
public: 
  Gizmos() = default; 

  Gizmos(const Gizmos&) = delete; 
  Gizmos& operator=(const Gizmos&) = delete;
  
  Gizmos(Gizmos&&) noexcept = default; 
  Gizmos& operator=(Gizmos&&) noexcept = default;
  
  ~Gizmos() = default;
  
  void initialize(const vec3f &pmin, const vec3f &pmax); 

  void render_xy_grid_gizmo(const mat4f& view_projection, float scene_radius) const; 
  void render_world_axis_gizmo(const mat4f &view, float aspect_ratio, int framebuffer_width, int framebuffer_height) const; 
  void render_clipping_plane_gizmo(const mat4f &model, const mat4f &view_projection) const; 

private: 
  void init_xy_gizmo(const vec3f &pmin, const vec3f &pmax); 
  void init_world_axis_gizmo(); 
  void init_clipping_plane_gizmo(const vec3f &pmin, const vec3f &pmax); 

  void init_arrow_shader_program(); 
  void init_grid_shader_program(); 
  void init_plane_shader_program(); 

private: 
  Line_renderer xy_grid_gizmo_{}; 
  Line_renderer xy_axis_gizmo_{}; 
  Line_renderer world_axis_gizmo_{}; 
  Line_renderer clipping_plane_gizmo_{};

  Shader_program shader_program_arrow_{0}; 
  Shader_program shader_program_grid_{0}; 
  Shader_program shader_program_plane_{0}; 

  // Sphere_renderer pivot_point_gizmo_{};
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Gizmos_impl.h"
#endif

#endif // CGAL_GLFW_INTERNAL_GIZMOS_H
