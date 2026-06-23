#ifndef CGAL_GLFW_INTERNAL_SCENE_RENDERER_H
#define CGAL_GLFW_INTERNAL_SCENE_RENDERER_H

#include <cstddef>
#include <vector>

#include <CGAL/Graphics_scene.h>
#include <CGAL/GLFW/bv_settings.h>

#include "Drawable.h"
#include "Gizmos.h"
#include "Shader_program.h"
#include <CGAL/GLFW/internal/controls/Camera_controller.h>
#include <CGAL/GLFW/internal/controls/Clipping_plane.h>
#include <CGAL/GLFW/internal/math/math_utils.h>
#include <CGAL/GLFW/internal/Window.h>


namespace CGAL {

namespace GLFW {
namespace internal {

class Scene_renderer {
public: 
  enum class Rendering_mode { DRAW_ALL = -1, DRAW_INSIDE_ONLY, DRAW_OUTSIDE_ONLY };

public: 
  Scene_renderer(const Graphics_scene& scene, 
                 const Camera_controller&         camera, 
                 const Clipping_plane& clip, 
                 const Window&         window); 

public: // public API
  // ---- per-frame entry point ----
  void render(); 

  // ---- vertex buffer loads ----
  void reload_faces_normals();
  
  // ---- state setters ----
  void flat_shading(bool b) { flat_shading_ = b; reload_faces_normals(); }
  void reverse_normal(bool b) { inverse_normal_ = b; reload_faces_normals(); }
  void use_mono_color(bool b) { use_mono_color_ = b; }
  void use_mono_color_normal(bool b) { use_mono_color_normal_ = b; }
  void display_face_normal(bool b) { display_face_normal_ = b; }
  
  void toggle_flat_shading() { flat_shading_ = !flat_shading_; reload_faces_normals(); }
  void toggle_reverse_normal() { inverse_normal_ = !inverse_normal_; reload_faces_normals(); }
  void toggle_use_mono_color() { use_mono_color_ = !use_mono_color_; }
  void toggle_use_mono_color_normal() { use_mono_color_normal_ = !use_mono_color_normal_; }
  void toggle_display_face_normal() { display_face_normal_= !display_face_normal_; }

  void draw_vertices(bool b) { draw_vertices_ = b; }
  void draw_edges(bool b) { draw_edges_ = b; }
  void draw_faces(bool b) { draw_faces_ = b; }
  void draw_rays(bool b) { draw_rays_ = b; }
  void draw_lines(bool b) { draw_lines_ = b; }
  void draw_cylinder_edge(bool b) { draw_cylinder_edge_ = b; }
  void draw_sphere_vertex(bool b) { draw_sphere_vertex_ = b; }
  void draw_normals(bool b) { draw_normals_ = b; }
  void draw_mesh_triangles(bool b) { draw_mesh_triangles_ = b; }
  void draw_clipping_plane(bool b) { draw_clipping_plane_ = b; }
  void draw_world_axis(bool b) { draw_world_axis_ = b; }
  void draw_xy_grid(bool b) { draw_xy_grid_ = b; }

  void toggle_draw_vertices() { draw_vertices_ = !draw_vertices_; }
  void toggle_draw_edges() { draw_edges_ = !draw_edges_; }
  void toggle_draw_faces() { draw_faces_ = !draw_faces_; }
  void toggle_draw_rays() { draw_rays_ = !draw_rays_; }
  void toggle_draw_lines() { draw_lines_ = !draw_lines_; }
  void toggle_draw_cylinder_edge() { draw_cylinder_edge_ = !draw_cylinder_edge_; }
  void toggle_draw_sphere_vertex() { draw_sphere_vertex_ = !draw_sphere_vertex_; }
  void toggle_draw_normals() { draw_normals_ = !draw_normals_; }
  void toggle_draw_mesh_triangles() { draw_mesh_triangles_ = !draw_mesh_triangles_; }
  void toggle_draw_clipping_plane() { draw_clipping_plane_ = !draw_clipping_plane_; }
  void toggle_draw_world_axis() { draw_world_axis_ = !draw_world_axis_; }
  void toggle_draw_xy_grid() { draw_xy_grid_ = !draw_xy_grid_; }

  void size_rays(float s) { size_rays_ = s; }
  void size_lines(float s) { size_lines_ = s; }
  void normal_height_factor(float h) { normal_height_factor_ = h; }

  void light_position(const vec4f& p) { light_position_ = p; }
  void light_ambient(const vec4f& c) { ambient_color_ = c; }
  void light_diffuse(const vec4f& c) { diffuse_color_ = c; }
  void light_specular(const vec4f& c) { specular_color_ = c; }
  void light_shininess(float s) { shininess_ = s; }
  void default_color_normals(const CGAL::IO::Color& c) { default_color_normal_ = c; }

  void scene_scale(float s); 

  // ---- state getters ----
  bool flat_shading() const { return flat_shading_; }
  bool reverse_normal() const { return inverse_normal_; }
  bool use_mono_color() const { return use_mono_color_; }
  bool use_mono_color_normal() const { return use_mono_color_normal_; }
  bool display_face_normal() const { return display_face_normal_; }
  
  bool draw_vertices() const { return draw_vertices_; }
  bool draw_edges() const { return draw_edges_; }
  bool draw_faces() const { return draw_faces_; }
  bool draw_rays() const { return draw_rays_; }
  bool draw_lines() const { return draw_lines_; }
  bool draw_cylinder_edge() const { return draw_cylinder_edge_; }
  bool draw_sphere_vertex() const { return draw_sphere_vertex_; }
  bool draw_normals() const { return draw_normals_; }
  bool draw_mesh_triangles() const { return draw_mesh_triangles_; }
  bool draw_clipping_plane() const { return draw_clipping_plane_; }
  bool draw_world_axis() const { return draw_world_axis_; }
  bool draw_xy_grid() const { return draw_xy_grid_; }

  float size_vertices() const { return size_vertices_; }
  float size_edges() const { return size_edges_; }
  float size_rays() const { return size_rays_; }
  float size_lines() const { return size_lines_; }
  float normal_height_factor() const { return normal_height_factor_; }

  const vec4f& light_position() const { return light_position_; }
  const vec4f& light_ambient() const { return ambient_color_; }
  const vec4f& light_diffuse() const { return diffuse_color_; }
  const vec4f& light_specular() const { return specular_color_; }
  float light_shininess() const { return shininess_; }

  const CGAL::IO::Color& default_color_normals() const { return default_color_normal_; }

  // ---- Light and size adjustments ----

  void inc_light_all(float dt);
  void inc_red_comp(float dt);
  void inc_green_comp(float dt);
  void inc_blue_comp(float dt);

  void inc_size_edge(float dt);
  void inc_size_vertex(float dt);

private: // private member methods
  // ---- pipeline ----
  void compile_shaders(); 
  void load_all();             
  void check_geometry_feature_availability() const; 
  void compute_mvp(); 

  // ---- per-shader uniforms ----
  void update_face_uniforms();  
  void update_sphere_uniforms();  
  void update_cylinder_uniforms();  
  void update_pl_uniforms(const vec3f& default_color = {0,0,0});  
  void update_line_uniforms(float size, const vec3f& default_color = {0,0,0});  
  void update_normals_uniforms(); 
  void update_triangles_uniforms(); 

  // ---- passes ----
  void render_vertices(); 
  void render_edges(); 
  void render_rays(); 
  void render_lines(); 
  void render_faces(); 
  void render_faces_bis(Rendering_mode mode); 
  void render_normals(); 
  void render_mesh_triangles(); 
  void render_clipping_plane(); 
  void render_xy_grid(); 
  void render_world_axis(); 

  // ---- helpers ----
                                            
  static vec3f color_to_normalized_vec3(const CGAL::IO::Color& c); 
  static vec4f color_to_normalized_vec4(const CGAL::IO::Color& c); 

private: // private member attributes
  // ---- references (lifetime owned by Basic_viewer) ----  
  const Graphics_scene& scene_;
  const Camera_controller& camera_;
  const Clipping_plane& clipping_plane_;
  const Window& window_;

  // ---- GPU resources ----  
  Shader_program shader_pl_{0};
  Shader_program shader_sphere_{0};
  Shader_program shader_line_{0};
  Shader_program shader_cylinder_{0};
  Shader_program shader_face_{0};
  Shader_program shader_mesh_triangle_{0};
  Shader_program shader_normal_{0};

  Drawable points_{}; 
  Drawable segments_{}; 
  Drawable rays_{}; 
  Drawable lines_{}; 
  Drawable faces_{}; 

  Gizmos gizmos_{}; 

  // ---- frame matrices (recomputed each frame) ----  
  mat4f view_matrix_{mat4f::Identity()};
  mat4f projection_matrix_{mat4f::Identity()};
  mat4f view_projection_matrix_{mat4f::Identity()};
  vec4f point_plane_{0,0,0,1}; 
  vec4f normal_plane_{0,0,1,0}; 

  // ---- visibility toggles ----  
  bool draw_vertices_{false}, draw_edges_{true}, draw_faces_{true}; 
  bool draw_rays_{true}, draw_lines_{true}; 
  bool draw_world_axis_{true}, draw_xy_grid_{false}, draw_clipping_plane_{true}; 
  bool draw_mesh_triangles_{false}, draw_normals_{false}; 
  bool draw_cylinder_edge_{false}, draw_sphere_vertex_{false}; 

  // ---- shading / coloring ----
  bool flat_shading_{true}, inverse_normal_{false}; 
  bool use_mono_color_{false}, use_mono_color_normal_{false}; 
  bool display_face_normal_{false};

  // ---- sizes ----
  float size_vertices_; 
  float size_edges_; 
  float size_rays_{CGAL_GLFW_SIZE_RAYS}; 
  float size_lines_{CGAL_GLFW_SIZE_LINES}; 
  float size_normals_; 
  float normal_height_factor_; 

  float scene_scale_{1.0f};

  static constexpr float MIN_FRAC_VERTEX = 0.001f;
  static constexpr float MAX_FRAC_VERTEX = 0.005f;
  static constexpr float MIN_FRAC_EDGE   = 0.0002f;
  static constexpr float MAX_FRAC_EDGE   = 0.0006f;
  static constexpr float MIN_FRAC_NORMAL = 0.0002f; 
  static constexpr float MAX_FRAC_NORMAL = 0.0006f; 
  static constexpr float MIN_FRAC_NORMAL_LENGTH = 0.05f;
  static constexpr float MAX_FRAC_NORMAL_LENGTH = 0.08f;
  static constexpr float SIZE_SWEEP_SECONDS = 2.0f;

  // ---- light + colors ----
  vec4f light_position_{CGAL_GLFW_LIGHT_POSITION}; 
  vec4f ambient_color_{CGAL_GLFW_AMBIENT_COLOR}; 
  vec4f diffuse_color_{CGAL_GLFW_DIFFUSE_COLOR}; 
  vec4f specular_color_{CGAL_GLFW_SPECULAR_COLOR}; 
  float shininess_{CGAL_GLFW_SHININESS}; 

  CGAL::IO::Color default_color_normal_ = CGAL_GLFW_NORMALS_MONO_COLOR;
  vec3f default_color_point_; 
  vec3f default_color_segment_; 
  vec3f default_color_ray_; 
  vec3f default_color_line_; 
  vec3f default_color_face_; 

  // ---- caps ----
  mutable bool geometry_feature_enabled_{true}; 
  
  bool is_opengl_4_3_{false}; 
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY 
#include "Scene_renderer_impl.h"
#endif

#endif // CGAL_GLFW_INTERNAL_SCENE_RENDERER_H
