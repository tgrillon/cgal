// Copyright (c) 2018  GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Théo Benard <benard320@gmail.com>
//                 Théo Grillon <theogrillon6f9@gmail.com>

#ifndef CGAL_GLFW_BASIC_VIEWER_H
#define CGAL_GLFW_BASIC_VIEWER_H

#include <iostream>
#include <memory>
#include <stdlib.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <CGAL/Aff_transformation_3.h>
#include <CGAL/Basic_shaders.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Graphics_scene.h>
#include <CGAL/Plane_3.h>

#include <CGAL/GLFW/bv_settings.h>

#include <CGAL/GLFW/internal/Animation_controller.h>
#include <CGAL/GLFW/internal/Camera.h>
#include <CGAL/GLFW/internal/Clipping_plane.h>
#include <CGAL/GLFW/internal/Window.h>
#include <CGAL/GLFW/internal/Input.h>
#include <CGAL/GLFW/internal/Line_renderer.h>
#include <CGAL/GLFW/internal/Shader.h>
#include <CGAL/GLFW/internal/event.h>
#include <CGAL/GLFW/internal/stb_init.h>
#include <CGAL/GLFW/internal/utils.h>

namespace CGAL {
namespace GLFW {

// forward declaration
namespace internal {
  class Action_registry; 
}

struct Basic_viewer_options {
  bool init_draw_vertices = false; 
  bool init_draw_edges = true;
  bool init_draw_faces = true; 
  bool init_draw_rays = true;
  bool init_draw_lines = true; 
  bool init_use_mono_color = false;
  bool init_inverse_normal = false; 
  bool init_flat_shading = true;
  bool hidden_window = false;
};

class Basic_viewer {
public:
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Local_kernel;

  enum class Rendering_mode {
    DRAW_ALL = -1,
    DRAW_INSIDE_ONLY,
    DRAW_OUTSIDE_ONLY
  };

  using CP_Display_mode = internal::Clipping_plane::Display_mode;

  /***** Lifecycle ****/

  Basic_viewer(const Graphics_scene &graphic_scene,
               const char *title = "CGAL Basic Viewer (GLFW)", 
               const Basic_viewer_options& opts = {});

  ~Basic_viewer();

  void show();
  void make_screenshot(const std::string &file_path);

  /***** Window ****/

  inline void two_dimensional() { camera_->set_orthographic(); }

  inline bool is_orthographic() const { return camera_->is_orthographic(); }
  inline bool is_two_dimensional() const {
    return !is_orthographic() && scene_.is_two_dimensional();
  }

  /***** Element sizes ****/

  inline void size_vertices(float s) { size_vertices_ = s; }
  inline void size_edges(float s) { size_edges_ = s; }
  inline void size_rays(float s) { size_rays_ = s; }
  inline void size_lines(float s) { size_lines_ = s; }
  inline void normal_height_factor(float h) { normal_height_factor_ = h; }

  inline float size_vertices() const { return size_vertices_; }
  inline float size_edges() const { return size_edges_; }
  inline float size_rays() const { return size_rays_; }
  inline float size_lines() const { return size_lines_; }

  /***** Lights ****/

  inline void light_position(const vec4f &p) { light_position_ = p; }
  inline void light_ambient(const vec4f &c) { ambient_color_ = c; }
  inline void light_diffuse(const vec4f &c) { diffuse_color_ = c; }
  inline void light_specular(const vec4f &c) { specular_color_ = c; }
  inline void light_shininess(float s) { shininess_ = s; }

  inline vec4f light_position() const { return light_position_; }
  inline vec4f light_ambient() const { return ambient_color_; }
  inline vec4f light_diffuse() const { return diffuse_color_; }
  inline vec4f light_specular() const { return specular_color_; }
  inline float light_shininess() const { return shininess_; }

  /***** Drawing flags ****/

  inline void draw_vertices(bool b) { draw_vertices_ = b; }
  inline void draw_edges(bool b) { draw_edges_ = b; }
  inline void draw_rays(bool b) { draw_rays_ = b; }
  inline void draw_lines(bool b) { draw_lines_ = b; }
  inline void draw_faces(bool b) { draw_faces_ = b; }
  inline void draw_clipping_plane(bool b) { draw_clipping_plane_ = b; }
  inline void draw_world_axis(bool b) { draw_world_axis_ = b; }
  inline void draw_xy_grid(bool b) { draw_xy_grid_ = b; }
  inline void draw_mesh_triangles(bool b) { draw_mesh_triangles_ = b; }

  inline void toggle_draw_vertices() { draw_vertices_ = !draw_vertices_; }
  inline void toggle_draw_edges() { draw_edges_ = !draw_edges_; }
  inline void toggle_draw_rays() { draw_rays_ = !draw_rays_; }
  inline void toggle_draw_lines() { draw_lines_ = !draw_lines_; }
  inline void toggle_draw_faces() { draw_faces_ = !draw_faces_; }
  inline void toggle_draw_clipping_plane() {
    draw_clipping_plane_ = !draw_clipping_plane_;
  }
  inline void toggle_draw_world_axis() { draw_world_axis_ = !draw_world_axis_; }
  inline void toggle_draw_xy_grid() { draw_xy_grid_ = !draw_xy_grid_; }
  inline void toggle_draw_mesh_triangles() {
    draw_mesh_triangles_ = !draw_mesh_triangles_;
  }
  inline void toggle_draw_normals() { draw_normals_ = !draw_normals_; }
  inline void toggle_draw_cylinder_edge() {
    draw_cylinder_edge_ = !draw_cylinder_edge_;
  }
  inline void toggle_draw_sphere_vertex() {
    draw_sphere_vertex_ = !draw_sphere_vertex_;
  }

  inline bool draw_vertices() const { return draw_vertices_; }
  inline bool draw_edges() const { return draw_edges_; }
  inline bool draw_rays() const { return draw_rays_; }
  inline bool draw_lines() const { return draw_lines_; }
  inline bool draw_faces() const { return draw_faces_; }
  inline bool draw_world_axis() const { return draw_world_axis_; }
  inline bool draw_xy_grid() const { return draw_xy_grid_; }
  inline bool draw_mesh_triangles() const { return draw_mesh_triangles_; }

  /***** Color and shading ****/

  inline void use_mono_color(bool b) { use_mono_color_ = b; }
  inline void use_mono_color_normals(bool b) { use_mono_color_normal_ = b; }
  inline void flat_shading(bool b) { flat_shading_ = b; }
  inline void reverse_normal(bool b) { inverse_normal_ = b; }
  inline void default_color_normals(const CGAL::IO::Color &c) {
    default_color_normal_ = c;
  }

  inline void toggle_use_mono_color() { use_mono_color_ = !use_mono_color_; }
  inline void toggle_use_mono_color_normals() {
    use_mono_color_normal_ = !use_mono_color_normal_;
  }
  inline void toggle_display_face_normal() {
    display_face_normal_ = !display_face_normal_;
  }
  inline void toggle_reverse_normal() { inverse_normal_ = !inverse_normal_; }
  inline void toggle_flat_shading() {
    flat_shading_ = !flat_shading_;
    are_buffers_initialized_ = false;
  }

  inline bool use_mono_color() const { return use_mono_color_; }
  inline bool use_mono_color_normal() const { return use_mono_color_normal_; }
  inline bool reverse_normal() const { return inverse_normal_; }
  inline bool flat_shading() const { return flat_shading_; }

  // Toggles the inverse-normal flag, mirrors it onto the scene normals, and
  // invalidates the rendering buffers so the change takes visual effect.
  inline void reverse_all_normals() {
    inverse_normal_ = !inverse_normal_;
    scene_.reverse_all_normals();
    are_buffers_initialized_ = false;
  }

  /***** Application state ****/

  inline void print_application_state(bool b) { print_application_state_ = b; }
  inline void toggle_print_application_state() {
    print_application_state_ = !print_application_state_;
    std::cout << "\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K"
                << "\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\r" << std::flush;
  }
  inline bool print_application_state() const {
    return print_application_state_;
  }

  /***** Camera and scene ****/

  inline void scene_radius(float r) { camera_->set_radius(r); }
  inline void scene_center(const vec3f &c) { camera_->set_center(c); }
  inline void scene_center(float x, float y, float z) {
    camera_->set_center({x, y, z});
  }

  inline void camera_position(const vec3f &p) { camera_->set_position(p); }
  inline void camera_position(float x, float y, float z) {
    camera_->set_position({x, y, z});
  }
  inline void camera_orientation(const vec3f &f, float u) {
    camera_->set_orientation(f, u);
  }
  inline void zoom(float z) { camera_->set_size(z); }

  inline void align_camera_to_clipping_plane() {
    camera_->align_to_plane(clipping_plane_->get_normal());
  }

  inline vec3f position() const { return camera_->get_position(); }
  inline vec3f forward() const { return camera_->get_forward(); }
  inline vec3f right() const { return camera_->get_right(); }
  inline vec3f up() const { return camera_->get_up(); }

  /***** Clipping plane ****/

  inline void display_mode(CP_Display_mode m) {
    if (clipping_plane_) clipping_plane_->display_mode(m);
  }
  inline void clipping_plane_orientation(const vec3f &n) {
    clipping_plane_->set_orientation(n);
  }
  inline void clipping_plane_translate_along_normal(float t) {
    clipping_plane_->translation(t * .1);
  }
  inline void clipping_plane_translate_along_camera_forward(float t) {
    clipping_plane_->translation(camera_->get_forward(), t * .1);
  }

  inline bool clipping_plane_enabled() const {
    return clipping_plane_->display_mode_enabled();
  }

  CGAL::Plane_3<Local_kernel> clipping_plane() const;

  /***** Animation ****/

  inline void animation_duration(std::chrono::milliseconds duration) {
    animation_controller_->set_duration(duration);
  }

  /***** Scene ****/

  inline const Graphics_scene &graphics_scene() const { return scene_; }

private:

  /***** Setup ****/

  bool setup_context(bool hidden);
  bool setup_inputs();

  void compile_shaders();
  void load_buffer(int i, int location, int gs_enum, int data_count);
  void load_buffer(int i, int location, const std::vector<float> &vector,
                   int data_count);
  void initialize_buffers();
  void initialize_camera();
  void initialize_and_load_world_axis();
  void initialize_and_load_clipping_plane();
  void load_scene();

  /***** Action registration ****/

  void register_actions() const;
  void register_clipping_plane_actions() const;
  void register_application_actions() const;
  void register_window_actions() const;
  void register_camera_actions() const;
  void register_scene_actions() const;
  void register_light_actions() const;
  void register_animation_actions() const;

  /***** Action handlers (extracted from multi-statement lambdas) ****/

  void zoom_camera(float scroll_y);
  void change_camera_fov(float scroll_y);
  void reset_camera();
  void reset_camera_and_clipping_plane();

  void rotate_clipping_plane(float dx, float dy);
  void translate_clipping_plane(float dx, float dy);
  void translate_clipping_plane_along_camera(float dx, float dy);
  void reset_clipping_plane();

  void save_key_frame();
  void run_or_stop_animation();

  /***** Light and size adjustments ****/

  void increase_light_all(const float delta_time);
  void increase_red_component(const float delta_time);
  void increase_green_component(const float delta_time);
  void increase_blue_component(const float delta_time);

  void increase_size_edge(const float delta_time);
  void decrease_size_edge(const float delta_time);
  void increase_size_vertex(const float delta_time);
  void decrease_size_vertex(const float delta_time);

  /***** Render loop ****/

  void compute_model_view_projection_matrix(const float delta_time);

  void update_face_uniforms();
  void update_sphere_uniforms();
  void update_cylinder_uniforms();
  void update_pl_uniforms(const vec3f &default_color = {0, 0, 0});
  void update_line_uniforms(float size, const vec3f &default_color = {0, 0, 0});
  void update_clipping_uniforms();
  void update_world_axis_uniforms();
  void update_xy_axis_uniforms();
  void update_xy_grid_uniforms();
  void update_normals_uniforms();
  void update_triangles_uniforms();

  void render_scene(const float delta_time);
  void draw(const float delta_time = 0);

  void render_rays();
  void render_edges();
  void render_lines();
  void render_vertices();
  void render_faces();
  void render_faces_bis(Rendering_mode mode);
  void render_world_axis();
  void render_xy_grid();
  void render_normals();
  void render_triangles();
  void render_clipping_plane();

  /***** Misc ****/

  void capture_screenshot(const std::string &file_path,
                          bool use_framebuffer = false);
  void print_application_state(float &elapsed_time, const float delta_time);
  void check_geometry_feature_availability();
  void change_pivot_point();
  bool need_update() const;
  void handle_events(float delta_time);

  std::vector<float> aggregating_data(int mono_enum, int colored_enum) const;
  std::vector<float> aggregating_color_data(int mono_enum, int colored_enum,
                                            const CGAL::IO::Color &mono_color) const;

  vec4f color_to_normalized_vec4(const CGAL::IO::Color &c) const;
  vec3f color_to_normalized_vec3(const CGAL::IO::Color &c) const;

private:
  std::unique_ptr<internal::Window> window_{nullptr};

  const Graphics_scene &scene_;

  const char *title_;
  bool draw_vertices_{false};
  bool draw_edges_{true};
  bool draw_faces_{true};
  bool draw_rays_{true};
  bool draw_lines_{true};
  bool draw_cylinder_edge_{false};
  bool draw_sphere_vertex_{false};
  bool draw_mesh_triangles_{false};

  bool display_face_normal_{false};
  bool use_mono_color_normal_{false};
  bool use_mono_color_{false};

  bool inverse_normal_{false};
  bool flat_shading_{true};

  bool print_application_state_{true};
  bool are_buffers_initialized_{false};

  bool draw_world_axis_{true};
  bool draw_xy_grid_{false};
  bool draw_normals_{false};

  bool is_opengl_4_3_{false};

  bool geometry_feature_enabled_{true};

  bool need_update_{true};

  internal::Line_renderer world_axis_renderer_;
  internal::Line_renderer xy_grid_renderer_;
  internal::Line_renderer xy_axis_renderer_;

  float size_vertices_{CGAL_SIZE_VERTICES};
  float size_edges_{CGAL_SIZE_EDGES};
  float size_rays_{CGAL_SIZE_RAYS};
  float size_lines_{CGAL_SIZE_LINES};
  float size_normals_{CGAL_SIZE_NORMALS};

  float normal_height_factor_{CGAL_NORMAL_HEIGHT_FACTOR};

  CGAL::IO::Color default_color_normal_ = CGAL_NORMALS_MONO_COLOR;

  vec4f light_position_{CGAL_LIGHT_POSITION};
  vec4f ambient_color_{CGAL_AMBIENT_COLOR};
  vec4f diffuse_color_{CGAL_DIFFUSE_COLOR};
  vec4f specular_color_{CGAL_SPECULAR_COLOR};

  float shininess_{CGAL_SHININESS};

  vec3f default_color_ray_;
  vec3f default_color_face_;
  vec3f default_color_line_;
  vec3f default_color_point_;
  vec3f default_color_segment_;

  vec4f clip_plane_{0, 0, 1, 0};
  vec4f point_plane_{0, 0, 0, 1};

  mat4f model_matrix_{mat4f::Identity()};
  mat4f view_matrix_{mat4f::Identity()};
  mat4f projection_matrix_{mat4f::Identity()};
  mat4f view_projection_matrix_{mat4f::Identity()};

  internal::Shader shader_face_;
  internal::Shader shader_sphere_;
  internal::Shader shader_line_;
  internal::Shader shader_pl_;
  internal::Shader shader_cylinder_;
  internal::Shader shader_plane_;
  internal::Shader shader_grid_;
  internal::Shader shader_normal_;
  internal::Shader shader_arrow_;
  internal::Shader shader_triangles_;

  float aspect_ratio_{1.f};

  float delta_time_{0};

  float last_x_{0.f};
  float last_y_{0.f};

  std::pair<vec3f, vec3f> bounding_box_;

  std::unique_ptr<internal::Camera> camera_{ nullptr };

  std::unique_ptr<internal::Clipping_plane> clipping_plane_{ nullptr };

  bool draw_clipping_plane_{true};

  std::unique_ptr<internal::Action_registry> action_registry_{ nullptr };
  std::unique_ptr<internal::Animation_controller> animation_controller_ { nullptr };

  enum Vao_enum {
    VAO_POINTS = 0,
    VAO_SEGMENTS,
    VAO_RAYS,
    VAO_LINES,
    VAO_FACES,
    NB_VAO_BUFFERS
  };

  unsigned int vao_[NB_VAO_BUFFERS];

  static const unsigned int NB_GL_BUFFERS =
      (Graphics_scene::END_POS - Graphics_scene::BEGIN_POS) +
      (Graphics_scene::END_COLOR - Graphics_scene::BEGIN_COLOR) + 2;

  unsigned int vbo_[NB_GL_BUFFERS];
};
} // end namespace GLFW

using GLFW::Basic_viewer;

inline void
draw_graphics_scene(const Graphics_scene &graphics_scene,
                    const char *title = "CGAL Basic Viewer (GLFW)") {
  Basic_viewer basic_viewer(graphics_scene, title);
  basic_viewer.show();
}

} // end namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include <CGAL/GLFW/Basic_viewer_impl.h>
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_BASIC_VIEWER_H
