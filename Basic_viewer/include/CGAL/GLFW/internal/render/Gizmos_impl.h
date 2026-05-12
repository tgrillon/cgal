#ifndef CGAL_GLFW_INTERNAL_GIZMOS_IMPL_H
#define CGAL_GLFW_INTERNAL_GIZMOS_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Gizmos.h"

#else 
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <glad/gl.h>

#include <CGAL/Basic_shaders.h>

#include <CGAL/GLFW/internal/math/math_utils.h>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION  
void Gizmos::initialize(const vec3f &pmin, const vec3f &pmax) {
  init_xy_gizmo(pmin, pmax); 
  init_world_axis_gizmo(); 
  init_clipping_plane_gizmo(pmin, pmax);

  init_arrow_shader_program(); 
  init_grid_shader_program(); 
  init_plane_shader_program(); 
} 

CGAL_INLINE_FUNCTION
void Gizmos::render_xy_grid_gizmo(const mat4f &view_projection, float scene_radius) const {
  CGAL_precondition_msg(xy_grid_gizmo_.is_valid() && xy_axis_gizmo_.is_valid(), 
    "Gizmos::initialize must be call before this method");
  if (!xy_grid_gizmo_.is_valid() || !xy_axis_gizmo_.is_valid()) return; 

  // Check if the shader program compiled and linked successfully
  if (shader_program_grid_.is_valid()) {
    shader_program_grid_.use();
    shader_program_grid_.uniform("u_Mvp", view_projection);

    xy_grid_gizmo_.draw(); 
  }
  
  if (shader_program_arrow_.is_valid()) {
    shader_program_arrow_.use();
    shader_program_arrow_.uniform("u_Mvp", view_projection);
    shader_program_arrow_.uniform("u_SceneRadius", scene_radius);

    xy_axis_gizmo_.draw();
  }
} 

CGAL_INLINE_FUNCTION
void Gizmos::render_world_axis_gizmo(const mat4f& view, float aspect_ratio, int framebuffer_width, int framebuffer_height) const {
  CGAL_precondition_msg(world_axis_gizmo_.is_valid(), 
    "Gizmos::initialize must be call before this method");
  if (!world_axis_gizmo_.is_valid()) return; 

  if (!shader_program_arrow_.is_valid()) return; 

  // Update uniforms

  // we only want the rotation part of the view matrix
  mat3f rotation = view.block<3, 3>(0, 0);

  mat4f rotation_4x4 = mat4f::Identity();
  rotation_4x4.block<3, 3>(0, 0) = rotation;

  float half_width = aspect_ratio * 0.1f;
  float half_height = 0.1f;
  mat4f projection = internal::utils::ortho(-half_width, half_width, -half_height,
                                  half_height, -1.0f, 1.0f);

  mat4f translation = internal::transform::translation(
      vec3f(half_width - 0.1f * aspect_ratio, half_height - 0.1f, 0.0f));

  mat4f mvp = projection * rotation_4x4 * translation;

  shader_program_arrow_.use();
  shader_program_arrow_.uniform("u_Mvp", mvp);
  shader_program_arrow_.uniform("u_SceneRadius", 1.0f);

  // Draw 

  glDepthFunc(GL_LEQUAL);

  int w = framebuffer_width;
  int h = framebuffer_height;

  // Draw on the top-right corner of the screen
  glViewport(w - w / 5, h - h / 5, w / 5, h / 5);
  world_axis_gizmo_.draw(); 

  // Restore the main viewport
  glViewport(0, 0, w, h);
} 

CGAL_INLINE_FUNCTION
void Gizmos::render_clipping_plane_gizmo(const mat4f& model, const mat4f& view_projection) const {
  CGAL_precondition_msg(clipping_plane_gizmo_.is_valid(), 
    "Gizmos::initialize must be call before this method");
  if (!clipping_plane_gizmo_.is_valid()) return; 

  if (!shader_program_plane_.is_valid()) return;
  
  if (shader_program_plane_.is_valid()) {
    shader_program_plane_.use();
    shader_program_plane_.uniform("u_Vp", view_projection);
    shader_program_plane_.uniform("u_M", model);
  }

  clipping_plane_gizmo_.draw(); 
} 

CGAL_INLINE_FUNCTION
void Gizmos::init_xy_gizmo(const vec3f &pmin, const vec3f &pmax) {
  float size = utils::distance(pmin, pmax) * 0.5; 
  { // XY grid axis initialization
    internal::Line_buffer buffer; 
    buffer.width(5.0f)
          .add_line({ .start=vec3f::Zero(), .end=size * vec3f::UnitX(), .color=vec3f(1, 0, 0) }) // x-axis
          .add_line({ .start=vec3f::Zero(), .end=size * vec3f::UnitY(), .color=vec3f(0, 1, 0) }); // y-axis

    xy_axis_gizmo_ = internal::Line_renderer::create(buffer);
  }
  
  { // XY grid initialization
    vec3f color(.8f, .8f, .8f);

    internal::Line_buffer buffer;
    buffer.width(2.0f)
          .add_line({ .start=vec3f::Zero(), .end=-2.f * size * vec3f::UnitX(), .color=color}) // -x-axis
          .add_line({ .start=vec3f::Zero(), .end=-2.f * size * vec3f::UnitY(), .color=color}) // -y-axis
          .add_line({ .start=vec3f::Zero(), .end=-2.f * size * vec3f::UnitZ(), .color=color}) // -z-axis
          .add_grid({ .size=size, .color=color });

    xy_grid_gizmo_ = internal::Line_renderer::create(buffer);
  }
} 
CGAL_INLINE_FUNCTION
void Gizmos::init_world_axis_gizmo() {
  internal::Line_buffer buffer; 
  buffer.width(3.0f)
        .add_line({ .start=vec3f::Zero(), .end=.1f * vec3f::UnitX(), .color=vec3f(1, 0, 0) }) // x-axis
        .add_line({ .start=vec3f::Zero(), .end=.1f * vec3f::UnitY(), .color=vec3f(0, 1, 0) }) // y-axis
        .add_line({ .start=vec3f::Zero(), .end=.1f * vec3f::UnitZ(), .color=vec3f(0, 0, 1) }); // z-axis

  world_axis_gizmo_ = internal::Line_renderer::create(buffer);
} 
CGAL_INLINE_FUNCTION
void Gizmos::init_clipping_plane_gizmo(const vec3f &pmin, const vec3f &pmax) {
  float size = ((pmax.x() - pmin.x()) +
                (pmax.y() - pmin.y()) +
                (pmax.z() - pmin.z()));
  
  Line_buffer buffer; 
  buffer.width(10.f)
        .add_line({ .start = {0.0, 0.0, 0.0}, .end = {0.0, 0.0, 1.0} })
        .add_grid({ .size = size, .subdivisions = 30 });

  clipping_plane_gizmo_ = Line_renderer::create(buffer); 
} 

CGAL_INLINE_FUNCTION
void Gizmos::init_arrow_shader_program() {
  const char *ARROW_VERTEX = VERTEX_SOURCE_LINE;
  const char *ARROW_GEOMETRY = GEOMETRY_SOURCE_ARROW;
  const char *ARROW_FRAGMENT = FRAGMENT_SOURCE_LINE;
  shader_program_arrow_ = internal::Shader_program::create(ARROW_VERTEX, ARROW_FRAGMENT, ARROW_GEOMETRY);
  if (!shader_program_arrow_.is_valid()) {
    std::cerr << "Failed to create arrow shader \n";
  }
}

CGAL_INLINE_FUNCTION
void Gizmos::init_grid_shader_program() {
  const char *GRID_VERTEX = VERTEX_SOURCE_LINE;
  const char *GRID_GEOMETRY = GEOMETRY_SOURCE_LINE;
  const char *GRID_FRAGMENT = FRAGMENT_SOURCE_LINE;
  shader_program_grid_ = internal::Shader_program::create(GRID_VERTEX, GRID_FRAGMENT, GRID_GEOMETRY);
  if (!shader_program_grid_.is_valid()) {
    std::cerr << "Failed to create grid shader program\n";
  }
}

CGAL_INLINE_FUNCTION
void Gizmos::init_plane_shader_program() {
  const char *PLANE_VERTEX = VERTEX_SOURCE_CLIPPING_PLANE;
  const char *PLANE_FRAGMENT = FRAGMENT_SOURCE_CLIPPING_PLANE;
  shader_program_plane_ = internal::Shader_program::create(PLANE_VERTEX, PLANE_FRAGMENT);
  if (!shader_program_plane_.is_valid()) {
    std::cerr << "Failed to create plane shader program\n";
  }
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_GIZMOS_IMPL_H
