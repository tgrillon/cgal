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

#include <CGAL/GLFW/internal/controls/Animation_controller.h>
#include <CGAL/GLFW/internal/controls/Clipping_plane.h>
#include <CGAL/GLFW/internal/controls/Camera.h>
#include <CGAL/GLFW/internal/input/Action_registry.h>
#include <CGAL/GLFW/internal/input/Input.h>
#include <CGAL/GLFW/internal/render/Drawable.h>
#include <CGAL/GLFW/internal/render/Gl_buffer.h>
#include <CGAL/GLFW/internal/render/Gizmos.h>
#include <CGAL/GLFW/internal/render/Line_renderer.h>
#include <CGAL/GLFW/internal/render/Shader_program.h>
#include <CGAL/GLFW/internal/render/Scene_renderer.h>
#include <CGAL/GLFW/internal/render/Vertex_array.h>
#include <CGAL/GLFW/internal/math/math_types.h>
#include <CGAL/GLFW/internal/math/math_utils.h>
#include <CGAL/GLFW/internal/stb_init.h>
#include <CGAL/GLFW/internal/Window.h>

namespace CGAL {
namespace GLFW {

struct Basic_viewer_options {
  bool hidden_window = false;
  bool draw_vertices = false; 
  bool draw_edges = true;
  bool draw_faces = true; 
  bool draw_rays = true;
  bool draw_lines = true; 
  bool use_mono_color = false;
  bool inverse_normal = false; 
  bool flat_shading = true;
};

class Basic_viewer {
public:
  typedef CGAL::Exact_predicates_inexact_constructions_kernel Local_kernel;

  using CP_Display_mode = internal::Clipping_plane::Clipping_mode;

  // ---- Lifecycle ----

  Basic_viewer(const Graphics_scene &scene,
               const char *title = "CGAL Basic Viewer (GLFW)", 
               const Basic_viewer_options& opts = {});

  ~Basic_viewer() = default;

  void show();
  void make_screenshot(const std::string &file_path);

  // ---- Window ----

  void two_dimensional() { camera_.set_orthographic(); }

  bool is_orthographic() const { return camera_.is_orthographic(); }
  bool is_two_dimensional() const {
    return !is_orthographic() && scene_.is_two_dimensional();
  }

  // ---- Element sizes ----

  void size_rays(float s) { renderer_.size_rays(s); }
  void size_lines(float s) { renderer_.size_lines(s); }

  float size_vertices() const { return renderer_.size_vertices(); }
  float size_edges() const { return renderer_.size_edges(); }
  float size_rays() const { return renderer_.size_rays(); }
  float size_lines() const { return renderer_.size_lines(); }

  // ---- Lights ----

  void light_position(const vec4f &p) { renderer_.light_position(p); }
  void light_ambient(const vec4f &c) { renderer_.light_ambient(c); }
  void light_diffuse(const vec4f &c) { renderer_.light_diffuse(c); }
  void light_specular(const vec4f &c) { renderer_.light_specular(c); }
  void light_shininess(float s) { renderer_.light_shininess(s); }

  vec4f light_position() const { return renderer_.light_position(); }
  vec4f light_ambient() const { return renderer_.light_ambient(); }
  vec4f light_diffuse() const { return renderer_.light_diffuse(); }
  vec4f light_specular() const { return renderer_.light_specular(); }
  float light_shininess() const { return renderer_.light_shininess(); }

  // ---- Drawing flags ----

  void draw_vertices(bool b) { renderer_.draw_vertices(b); }
  void draw_edges(bool b) { renderer_.draw_edges(b); }
  void draw_rays(bool b) { renderer_.draw_rays(b); }
  void draw_lines(bool b) { renderer_.draw_lines(b); }
  void draw_faces(bool b) { renderer_.draw_faces(b); }
  void draw_clipping_plane(bool b) { renderer_.draw_clipping_plane(b); }
  void draw_world_axis(bool b) { renderer_.draw_world_axis(b); }
  void draw_xy_grid(bool b) { renderer_.draw_xy_grid(b); }
  void draw_mesh_triangles(bool b) { renderer_.draw_mesh_triangles(b); }

  void toggle_draw_vertices() { renderer_.toggle_draw_vertices(); }
  void toggle_draw_edges() { renderer_.toggle_draw_edges(); }
  void toggle_draw_rays() { renderer_.toggle_draw_rays(); }
  void toggle_draw_lines() { renderer_.toggle_draw_lines(); }
  void toggle_draw_faces() { renderer_.toggle_draw_faces(); }
  void toggle_draw_clipping_plane() { renderer_.toggle_draw_clipping_plane(); }
  void toggle_draw_world_axis() { renderer_.toggle_draw_world_axis(); }
  void toggle_draw_xy_grid() { renderer_.toggle_draw_xy_grid(); }
  void toggle_draw_mesh_triangles() { renderer_.toggle_draw_mesh_triangles(); }
  void toggle_draw_normals() { renderer_.toggle_draw_normals(); }
  void toggle_draw_cylinder_edge() { renderer_.toggle_draw_cylinder_edge(); }
  void toggle_draw_sphere_vertex() { renderer_.toggle_draw_sphere_vertex(); }

  bool draw_vertices() const { return renderer_.draw_vertices(); }
  bool draw_edges() const { return renderer_.draw_edges(); }
  bool draw_rays() const { return renderer_.draw_rays(); }
  bool draw_lines() const { return renderer_.draw_lines(); }
  bool draw_faces() const { return renderer_.draw_faces(); }
  bool draw_world_axis() const { return renderer_.draw_world_axis(); }
  bool draw_xy_grid() const { return renderer_.draw_xy_grid(); }
  bool draw_mesh_triangles() const { return renderer_.draw_mesh_triangles(); }

  // ---- Color and shading ----

  void use_mono_color(bool b) { renderer_.use_mono_color(b); }
  void use_mono_color_normals(bool b) { renderer_.use_mono_color_normal(b); }
  void display_face_normal(bool b) { renderer_.display_face_normal(b); }
  void flat_shading(bool b) { renderer_.flat_shading(b); }
  void reverse_normal(bool b) { renderer_.reverse_normal(b); }
  void default_color_normals(const CGAL::IO::Color &c) { renderer_.default_color_normals(c); }

  void toggle_use_mono_color() { renderer_.toggle_use_mono_color(); }
  void toggle_use_mono_color_normals() { renderer_.toggle_use_mono_color_normal(); }
  void toggle_display_face_normal() { renderer_.toggle_display_face_normal(); }
  void toggle_reverse_normal() { renderer_.toggle_reverse_normal(); }
  void toggle_flat_shading() { renderer_.toggle_flat_shading(); }

  bool use_mono_color() const { return renderer_.use_mono_color(); }
  bool use_mono_color_normal() const { return renderer_.use_mono_color_normal(); }
  bool reverse_normal() const { return renderer_.reverse_normal(); }
  bool flat_shading() const { return renderer_.flat_shading(); }

  // Toggles the inverse-normal flag, mirrors it onto the scene normals, and
  // invalidates the rendering buffers so the change takes visual effect.
  void reverse_all_normals() {
    renderer_.toggle_reverse_normal();
    scene_.reverse_all_normals();
    renderer_.reload_faces_normals(); 
  }

  // ---- Application state ----

  void print_application_state(bool b) { print_application_state_ = b; }
  void toggle_print_application_state() {
    print_application_state_ = !print_application_state_;
    std::cout << "\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K\n\33[2K"
                << "\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\r" << std::flush;
  }
  bool print_application_state() const {
    return print_application_state_;
  }

  // ---- Camera and scene ----

  void scene_radius(float r) { camera_.set_radius(r); }
  void scene_center(const vec3f &c) { camera_.set_center(c); }
  void scene_center(float x, float y, float z) {
    camera_.set_center({x, y, z});
  }

  void camera_position(const vec3f &p) { camera_.set_position(p); }
  void camera_position(float x, float y, float z) {
    camera_.set_position({x, y, z});
  }
  void camera_orientation(const vec3f &f, float u) {
    camera_.set_orientation(f, u);
  }
  void zoom(float z) { camera_.set_size(z); }

  void align_camera_to_clipping_plane() {
    camera_.align_to_plane(clipping_plane_.normal());
  }

  vec3f position() const { return camera_.get_position(); }
  vec3f forward() const { return camera_.get_forward(); }
  vec3f right() const { return camera_.get_right(); }
  vec3f up() const { return camera_.get_up(); }

  // ---- Clipping plane ----

  void clipping_mode(CP_Display_mode m) {
    clipping_plane_.clipping_mode(m);
  }
  void clipping_plane_orientation(const vec3f &n) {
    clipping_plane_.orientation(n);
  }
  void clipping_plane_translate_along_normal(float t) {
    clipping_plane_.translation(t * .1);
  }
  void clipping_plane_translate_along_camera_forward(float t) {
    clipping_plane_.translation(camera_.get_forward(), t * .1);
  }

  bool clipping_plane_enabled() const {
    return clipping_plane_.clipping_enabled();
  }

  CGAL::Plane_3<Local_kernel> clipping_plane() const;

  // ---- Animation ----

  void animation_duration(std::chrono::milliseconds duration) {
    animation_controller_->set_duration(duration);
  }

  // ---- Scene ----

  const Graphics_scene &graphics_scene() const { return scene_; }

private:

  // ---- Setup ----

  void setup_context();
  void setup_inputs();

  void initialize_camera();

  // ---- Action registration ----

  void register_actions();
  void register_clipping_plane_actions(internal::Action_registry& registry);
  void register_application_actions(internal::Action_registry& registry);
  void register_window_actions(internal::Action_registry& registry);
  void register_camera_actions(internal::Action_registry& registry);
  void register_scene_actions(internal::Action_registry& registry);
  void register_light_actions(internal::Action_registry& registry);
  void register_animation_actions(internal::Action_registry& registry);

  // ---- Action handlers (extracted from multi-statement lambdas) ----

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

  // ---- Render loop ----

  void render_scene(float dt);

  // ---- Misc ----

  void print_application_state(float &elapsed_time, float dt);
  void change_pivot_point();
  bool need_update() const;
  void handle_events(float dt);

  // ---- Light and size adjustments ----

  void inc_light_all(float dt) { renderer_.inc_light_all(dt); }
  void inc_red_comp(float dt) { renderer_.inc_red_comp(dt); }
  void inc_green_comp(float dt) { renderer_.inc_green_comp(dt); }
  void inc_blue_comp(float dt) { renderer_.inc_blue_comp(dt); }

  void inc_size_edge(float dt) { renderer_.inc_size_edge(dt); }
  void inc_size_vertex(float dt) { renderer_.inc_size_vertex(dt); }

private:
  internal::Window window_;

  const Graphics_scene &scene_;

  bool print_application_state_{true};

  bool is_opengl_4_3_{false};

  bool need_update_{true};

  float last_x_{0.f};
  float last_y_{0.f};

  std::pair<vec3f, vec3f> bounding_box_;

  internal::Camera camera_{};
  internal::Clipping_plane clipping_plane_{};
  internal::Scene_renderer renderer_; 

  std::optional<internal::Action_registry> action_registry_{};
  std::optional<internal::Animation_controller> animation_controller_ {};
};
} // end namespace GLFW

using GLFW::Basic_viewer;

void
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
