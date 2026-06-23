#ifndef CGAL_GLFW_INTERNAL_SCENE_RENDERER_IMPL_H
#define CGAL_GLFW_INTERNAL_SCENE_RENDERER_IMPL_H

#include "Shader_program.h"
#include <CGAL/GLFW/Basic_viewer.h>
#include <CGAL/config.h>
#include <cmath>

#ifdef CGAL_HEADER_ONLY 
#define CGAL_INLINE_FUNCTION inline

#include "Scene_renderer.h"

#else 
#define CGAL_INLINE_FUNCTION 
#endif

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Scene_renderer::Scene_renderer(const Graphics_scene& scene,
                               const Camera_controller& camera,
                               const Clipping_plane& clip,
                               const Window&         window)
  : scene_(scene), camera_(camera), clipping_plane_(clip), window_(window) {

  // 1. Query GL capabilities (compile_shaders depends on is_opengl_4_3_).
  int opengl_major_version, opengl_minor_version;
  glGetIntegerv(GL_MAJOR_VERSION, &opengl_major_version);
  glGetIntegerv(GL_MINOR_VERSION, &opengl_minor_version);
  if (opengl_major_version > 4 ||
      (opengl_major_version == 4 && opengl_minor_version >= 3)) {
    is_opengl_4_3_ = true;
  }
  check_geometry_feature_availability();

  // 2. Compile shaders, then upload buffers.
  compile_shaders();
  load_all();

  vec3f pmin(scene_.bounding_box().xmin(), scene_.bounding_box().ymin(),
             scene_.bounding_box().zmin());

  vec3f pmax(scene_.bounding_box().xmax(), scene_.bounding_box().ymax(),
             scene_.bounding_box().zmax());
  gizmos_.initialize(pmin, pmax);

  default_color_ray_ = color_to_normalized_vec3(scene_.get_default_color_ray());
  default_color_face_ = color_to_normalized_vec3(scene_.get_default_color_face());
  default_color_line_ = color_to_normalized_vec3(scene_.get_default_color_line());
  default_color_point_ = color_to_normalized_vec3(scene_.get_default_color_point());
  default_color_segment_ = color_to_normalized_vec3(scene_.get_default_color_segment());
}

CGAL_INLINE_FUNCTION
void Scene_renderer::render() {
  compute_mvp(); 

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
  glEnable(GL_DEPTH_TEST); 
  glEnable(GL_PROGRAM_POINT_SIZE); 
  glEnable(GL_LINE_SMOOTH);
  
  if (draw_rays_) render_rays(); 
  if (draw_lines_) render_lines(); 
  if (draw_edges_) render_edges(); 
  if (draw_vertices_) render_vertices(); 
  if (clipping_plane_.clipping_enabled() && draw_clipping_plane_) render_clipping_plane(); 
  if (draw_normals_) render_normals(); 
  if (draw_faces_) render_faces(); 
  if (draw_mesh_triangles_) render_mesh_triangles(); 
  if (draw_world_axis_) render_world_axis(); 
  if (draw_xy_grid_) render_xy_grid(); 
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::load_all() {

  // 1) Point buffer

  points_.configure(GL_POINTS, {/*position=*/{.location=0, .components=3 }, 
                                   /*color=*/{.location=1, .components=3 }});
  points_.upload<float>(0, scene_.get_array_of_index(Graphics_scene::POS_POINTS));  
  points_.upload<float>(1, scene_.get_array_of_index(Graphics_scene::COLOR_POINTS));

  // 2) Segment buffer

  segments_.configure(GL_LINES, {/*position=*/{.location=0, .components=3 }, 
                                    /*color=*/{.location=1, .components=3 }});
  segments_.upload<float>(0, scene_.get_array_of_index(Graphics_scene::POS_SEGMENTS)); 
  segments_.upload<float>(1, scene_.get_array_of_index(Graphics_scene::COLOR_SEGMENTS));

  // 3) Rays buffer

  rays_.configure(GL_LINES, {/*position=*/{.location=0, .components=3 }, 
                                /*color=*/{.location=1, .components=3 }});
  rays_.upload<float>(0, scene_.get_array_of_index(Graphics_scene::POS_RAYS));
  rays_.upload<float>(1, scene_.get_array_of_index(Graphics_scene::COLOR_RAYS));

  // 4) Lines buffer

  lines_.configure(GL_LINES, {/*position=*/{.location=0, .components=3 }, 
                                 /*color=*/{.location=1, .components=3 }});
  lines_.upload<float>(0, scene_.get_array_of_index(Graphics_scene::POS_LINES)); 
  lines_.upload<float>(1, scene_.get_array_of_index(Graphics_scene::COLOR_LINES));

  // 5) Face buffer

  auto normal_buffer_index = flat_shading_ ? Graphics_scene::FLAT_NORMAL_FACES 
                                           : Graphics_scene::SMOOTH_NORMAL_FACES;
  
  faces_.configure(GL_TRIANGLES, {/*position=*/{.location=0, .components=3 }, 
                                    /*normal=*/{.location=1, .components=3 },
                                     /*color=*/{.location=2, .components=3 }});
  faces_.upload<float>(0, scene_.get_array_of_index(Graphics_scene::POS_FACES)); 
  faces_.upload<float>(1, scene_.get_array_of_index(normal_buffer_index)); 
  faces_.upload<float>(2, scene_.get_array_of_index(Graphics_scene::COLOR_FACES));
}

CGAL_INLINE_FUNCTION
void Scene_renderer::reload_faces_normals() {
  auto normal_buffer_index = flat_shading_ ? Graphics_scene::FLAT_NORMAL_FACES 
                                           : Graphics_scene::SMOOTH_NORMAL_FACES;
  faces_.upload<float>(1, scene_.get_array_of_index(normal_buffer_index)); 
}

CGAL_INLINE_FUNCTION
void Scene_renderer::scene_scale(float s) { 
  scene_scale_ = s; 
  auto mid = [s](float min, float max) { return min + (max - min) * 0.5f * s; };
  size_vertices_ = mid(MIN_FRAC_VERTEX, MAX_FRAC_VERTEX); 
  size_edges_ = mid(MIN_FRAC_EDGE, MAX_FRAC_EDGE); 
  size_normals_ = mid(MIN_FRAC_NORMAL, MAX_FRAC_NORMAL); 
  normal_height_factor_ = mid(MIN_FRAC_NORMAL_LENGTH, MAX_FRAC_NORMAL_LENGTH); 
}

CGAL_INLINE_FUNCTION
void Scene_renderer::inc_light_all(float dt) {
  inc_red_comp(dt);
  inc_green_comp(dt);
  inc_blue_comp(dt);
}

CGAL_INLINE_FUNCTION
void Scene_renderer::inc_red_comp(float dt) {
  ambient_color_.x() = std::clamp(ambient_color_.x() + dt, 0.0f, 1.0f);
  diffuse_color_.x() = std::clamp(diffuse_color_.x() + dt, 0.0f, 1.0f);
  specular_color_.x() = std::clamp(specular_color_.x() + dt, 0.0f, 1.0f);
}

CGAL_INLINE_FUNCTION
void Scene_renderer::inc_green_comp(float dt) {
  ambient_color_.y() = std::clamp(ambient_color_.y() + dt, 0.0f, 1.0f);
  diffuse_color_.y() = std::clamp(diffuse_color_.y() + dt, 0.0f, 1.0f);
  specular_color_.y() = std::clamp(specular_color_.y() + dt, 0.0f, 1.0f);
}

CGAL_INLINE_FUNCTION
void Scene_renderer::inc_blue_comp(float dt) {
  ambient_color_.z() = std::clamp(ambient_color_.z() + dt, 0.0f, 1.0f);
  diffuse_color_.z() = std::clamp(diffuse_color_.z() + dt, 0.0f, 1.0f);
  specular_color_.z() = std::clamp(specular_color_.z() + dt, 0.0f, 1.0f);
}

CGAL_INLINE_FUNCTION
void Scene_renderer::inc_size_edge(float dt) {
  const float low_bound = MIN_FRAC_EDGE * scene_scale_;  
  const float high_bound = MAX_FRAC_EDGE * scene_scale_;
  constexpr float INV_SWEEP_SECONDS = 1 / SIZE_SWEEP_SECONDS; 
  const float step = (low_bound + high_bound) * INV_SWEEP_SECONDS; 
  size_edges_ = std::clamp(size_edges_ + step * dt, low_bound, high_bound);
}

CGAL_INLINE_FUNCTION
void Scene_renderer::inc_size_vertex(float dt) {
  const float low_bound = MIN_FRAC_VERTEX * scene_scale_;  
  const float high_bound = MAX_FRAC_VERTEX * scene_scale_;
  constexpr float INV_SWEEP_SECONDS = 1 / SIZE_SWEEP_SECONDS; 
  const float step = (low_bound + high_bound) * INV_SWEEP_SECONDS; 
  size_vertices_ = std::clamp(size_vertices_ + step * dt, low_bound, high_bound);
}

CGAL_INLINE_FUNCTION
void Scene_renderer::compile_shaders() {
  const char *PL_VERTEX =
      is_opengl_4_3_ ? VERTEX_SOURCE_P_L : VERTEX_SOURCE_P_L_COMP;
  const char *PL_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_pl_ = internal::Shader_program::create(PL_VERTEX, PL_FRAGMENT);
  if (!shader_pl_.is_valid()) {
    std::cerr << "Failed to create pl shader program\n";
  }     

  const char *FACE_VERTEX =
      is_opengl_4_3_ ? VERTEX_SOURCE_COLOR : VERTEX_SOURCE_COLOR_COMP;
  const char *FACE_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_COLOR : FRAGMENT_SOURCE_COLOR_COMP;
  shader_face_ = internal::Shader_program::create(FACE_VERTEX, FACE_FRAGMENT);
  if (!shader_face_.is_valid()) {
    std::cerr << "Failed to create face shader program\n";
  }

  const char *SHAPE_VERTEX = VERTEX_SOURCE_SHAPE;
  const char *POINT_GEOMETRY = GEOMETRY_SOURCE_SPHERE;
  shader_sphere_ = internal::Shader_program::create(SHAPE_VERTEX, PL_FRAGMENT, POINT_GEOMETRY);
  if (!shader_sphere_.is_valid()) {
    std::cerr << "Failed to create sphere shader program\n";
  }

  const char *EDGE_VERTEX = VERTEX_SOURCE_SHAPE;
  const char *EDGE_GEOMETRY = GEOMETRY_SOURCE_CYLINDER;
  const char *EDGE_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_cylinder_ = internal::Shader_program::create(EDGE_VERTEX, PL_FRAGMENT, EDGE_GEOMETRY);
  if (!shader_cylinder_.is_valid()) {
    std::cerr << "Failed to create cylinder shader program\n";
  }

  const char *LINE_VERTEX = VERTEX_SOURCE_LINE_WIDTH;
  const char *LINE_GEOMETRY = GEOMETRY_SOURCE_LINE_WIDTH;
  const char *LINE_FRAGMENT = FRAGMENT_SOURCE_P_L;
  shader_line_ = internal::Shader_program::create(LINE_VERTEX, LINE_FRAGMENT, LINE_GEOMETRY);
  if (!shader_line_.is_valid()) {
    std::cerr << "Failed to create line shader program\n";
  }

  const char *NORMAL_VERTEX = VERTEX_SOURCE_NORMAL;
  const char *NORMAL_GEOMETRY = GEOMETRY_SOURCE_NORMAL;
  const char *NORMAL_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_normal_ =
      internal::Shader_program::create(NORMAL_VERTEX, NORMAL_FRAGMENT, NORMAL_GEOMETRY);
  if (!shader_normal_.is_valid()) {
    std::cerr << "Failed to create normal shader program\n";
  }

  const char *TRIANGLE_VERTEX = VERTEX_SOURCE_TRIANGLE;
  const char *TRIANGLE_GEOMETRY = GEOMETRY_SOURCE_TRIANGLE;
  const char *TRIANGLE_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_mesh_triangle_ = internal::Shader_program::create(TRIANGLE_VERTEX, TRIANGLE_FRAGMENT, TRIANGLE_GEOMETRY);
  if (!shader_mesh_triangle_.is_valid()) {
    std::cerr << "Failed to create mesh triangle shader program\n";
  }
}

CGAL_INLINE_FUNCTION
void Scene_renderer::check_geometry_feature_availability() const {
  int max_geometry_output_vertices = 0;
  glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_geometry_output_vertices);
  int max_geometry_output_components = 0;
  glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS,
                &max_geometry_output_components);

  if (max_geometry_output_vertices < 128 ||
      max_geometry_output_components < 1024) {
    std::cout << "Cylinder edge and sphere vertex feature disabled! "
                 "(max_geometry_output_vertices="
              << max_geometry_output_vertices
              << ", max_geometry_output_components="
              << max_geometry_output_components << ")\n";
    geometry_feature_enabled_ = false;
  }
}

CGAL_INLINE_FUNCTION
void Scene_renderer::compute_mvp() {
  view_matrix_ = camera_.view(); 
  projection_matrix_ = camera_.proj(); 
  view_projection_matrix_ = projection_matrix_ * view_matrix_; 

  const mat4f& cm = clipping_plane_.model_matrix();
  point_plane_ = cm * vec4f(0,0,0,1);
  normal_plane_ = cm * vec4f(0,0,1,0); 
}

CGAL_INLINE_FUNCTION
void Scene_renderer::update_face_uniforms() {
  if (shader_face_.is_valid()) {
    shader_face_.use();

    shader_face_.uniform("u_Mvp", view_projection_matrix_);
    shader_face_.uniform("u_Mv", view_matrix_);
    if (use_mono_color_) {
      shader_face_.uniform("u_UseDefaultColor", 1);
      shader_face_.uniform("u_DefaultColor", default_color_face_);
    } else {
      shader_face_.uniform("u_UseDefaultColor", 0);
    }

    shader_face_.uniform("u_LightPos", light_position_);
    shader_face_.uniform("u_LightDiff", diffuse_color_);
    shader_face_.uniform("u_LightSpec", specular_color_);
    shader_face_.uniform("u_LightAmb", ambient_color_);
    shader_face_.uniform("u_SpecPower", shininess_);

    shader_face_.uniform("u_ClipPlane", normal_plane_);
    shader_face_.uniform("u_PointPlane", point_plane_);
    shader_face_.uniform("u_RenderingTransparency", clipping_plane_.transparency());
  }
}  

CGAL_INLINE_FUNCTION
void Scene_renderer::update_sphere_uniforms() {
  if (shader_sphere_.is_valid()) {
    shader_sphere_.use();

    bool half = clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    if (use_mono_color_) {
      shader_sphere_.uniform("u_UseDefaultColor", 1);
      shader_sphere_.uniform("u_DefaultColor", default_color_point_);
    } else {
      shader_sphere_.uniform("u_UseDefaultColor", 0);
    }

    shader_sphere_.uniform("u_RenderingMode", static_cast<float>(mode));

    shader_sphere_.uniform("u_Mvp", view_projection_matrix_);
    shader_sphere_.uniform("u_ClipPlane", normal_plane_);
    shader_sphere_.uniform("u_PointPlane", point_plane_);

    shader_sphere_.uniform("u_Radius", size_vertices_);
  }
}  

CGAL_INLINE_FUNCTION
void Scene_renderer::update_cylinder_uniforms() {
  if (shader_cylinder_.is_valid()) {
    shader_cylinder_.use();

    bool half = clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    if (use_mono_color_) {
      shader_cylinder_.uniform("u_UseDefaultColor", 1);
      shader_cylinder_.uniform("u_DefaultColor",
                                 default_color_segment_);
    } else {
      shader_cylinder_.uniform("u_UseDefaultColor", 0);
    }

    shader_cylinder_.uniform("u_RenderingMode", static_cast<float>(mode));

    shader_cylinder_.uniform("u_Mvp", view_projection_matrix_);
    shader_cylinder_.uniform("u_ClipPlane", normal_plane_);
    shader_cylinder_.uniform("u_PointPlane", point_plane_);

    shader_cylinder_.uniform("u_Radius", size_edges_);
  }
}  

CGAL_INLINE_FUNCTION
void Scene_renderer::update_pl_uniforms(const vec3f& default_color) {
  if (shader_pl_.is_valid()) {
    shader_pl_.use();

    bool half = clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    const mat4f& P = camera_.proj(); 
    const float h = window_.framebuffer_height(); 
    const float px_per_world_unit = h * P(1, 1) * 0.5f;

    shader_pl_.uniform("u_Mvp", view_projection_matrix_);
    shader_pl_.uniform("u_PxPerWorldUnit", px_per_world_unit);
    shader_pl_.uniform("u_WorldRadius", size_vertices_);
    if (use_mono_color_) {
      shader_pl_.uniform("u_UseDefaultColor", 1);
      shader_pl_.uniform("u_DefaultColor", default_color);
    } else {
      shader_pl_.uniform("u_UseDefaultColor", 0);
    }

    shader_pl_.uniform("u_ClipPlane", normal_plane_);
    shader_pl_.uniform("u_PointPlane", point_plane_);
    shader_pl_.uniform("u_RenderingMode", static_cast<float>(mode));
  }
}  

CGAL_INLINE_FUNCTION
void Scene_renderer::update_line_uniforms(float size, const vec3f& default_color) {
  if (shader_line_.is_valid()) {
    shader_line_.use();

    bool half = clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    vec2f viewport = {static_cast<float>(window_.framebuffer_width()),
                      static_cast<float>(window_.framebuffer_height())};

    const mat4f& P = camera_.proj(); 
    const float h = window_.framebuffer_height(); 
    const float px_per_world_unit = h * P(1, 1) * 0.5f;

    shader_line_.uniform("u_Mvp", view_projection_matrix_);
    shader_line_.uniform("u_PxPerWorldUnit", px_per_world_unit);
    shader_line_.uniform("u_WorldRadius", size);
    if (use_mono_color_) {
      shader_line_.uniform("u_UseDefaultColor", 1);
      shader_line_.uniform("u_DefaultColor", default_color);
    } else {
      shader_line_.uniform("u_UseDefaultColor", 0);
    }

    shader_line_.uniform("u_Viewport", viewport);

    shader_line_.uniform("u_ClipPlane", normal_plane_);
    shader_line_.uniform("u_PointPlane", point_plane_);
    shader_line_.uniform("u_RenderingMode", static_cast<float>(mode));
  }
}  

CGAL_INLINE_FUNCTION
void Scene_renderer::update_normals_uniforms() {
  if (shader_normal_.is_valid()) {
    shader_normal_.use();

    bool half = clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    vec2f viewport = {static_cast<float>(window_.framebuffer_width()),
                      static_cast<float>(window_.framebuffer_height())};

    const mat4f& P = camera_.proj(); 
    const float h = window_.framebuffer_height(); 
    const float px_per_world_unit = h * P(1, 1) * 0.5f;
    
    shader_normal_.uniform("u_PxPerWorldUnit", px_per_world_unit);
    shader_normal_.uniform("u_WorldRadius", size_normals_);
    shader_normal_.uniform("u_Viewport", viewport);

    vec4f color = color_to_normalized_vec4(default_color_normal_);
    shader_normal_.uniform("u_Mv", view_matrix_);
    if (use_mono_color_normal_) {
      shader_normal_.uniform("u_UseDefaultColor", 1);
      shader_normal_.uniform("u_DefaultColor", color);
    } else {
      shader_normal_.uniform("u_UseDefaultColor", 0);
    }
    shader_normal_.uniform("u_Projection", projection_matrix_);
    shader_normal_.uniform("u_Factor", normal_height_factor_);
    if (display_face_normal_) {
      shader_normal_.uniform("u_DisplayFaceNormal", 1);
    } else {
      shader_normal_.uniform("u_DisplayFaceNormal", 0);
    }

    shader_normal_.uniform("u_ClipPlane", normal_plane_);
    shader_normal_.uniform("u_PointPlane", point_plane_);
    shader_normal_.uniform("u_RenderingMode", static_cast<float>(mode));
  }
}

CGAL_INLINE_FUNCTION
void Scene_renderer::update_triangles_uniforms() {
  if (shader_mesh_triangle_.is_valid()) {
    shader_mesh_triangle_.use();

    bool half = clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    shader_mesh_triangle_.uniform("u_Mvp", view_projection_matrix_);

    shader_mesh_triangle_.uniform("u_ClipPlane", normal_plane_);
    shader_mesh_triangle_.uniform("u_PointPlane", point_plane_);
    shader_mesh_triangle_.uniform("u_RenderingMode", static_cast<float>(mode));
  }
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_vertices() {
  update_pl_uniforms(default_color_point_);
  if (draw_sphere_vertex_ && geometry_feature_enabled_) {
    update_sphere_uniforms();
  }

  glDepthFunc(GL_LEQUAL);
  points_.draw(); 
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_edges() {
  update_line_uniforms(size_edges_, default_color_segment_);
  if (draw_cylinder_edge_ && geometry_feature_enabled_) {
    update_cylinder_uniforms();
  }

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_POLYGON_OFFSET_FILL); 
  glPolygonOffset(1.0f, 1.0f); 
  segments_.draw(); 
  glDisable(GL_POLYGON_OFFSET_FILL); 
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_rays() {
  update_pl_uniforms(default_color_ray_);

  shader_pl_.uniform("u_RenderingMode",
                       static_cast<float>(Rendering_mode::DRAW_ALL));

  glLineWidth(size_rays_);
  rays_.draw(); 
  glLineWidth(1.0);
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_lines() {
  update_pl_uniforms(default_color_line_);

  shader_pl_.uniform("u_RenderingMode",
                       static_cast<float>(Rendering_mode::DRAW_ALL));

  glLineWidth(size_lines_);
  lines_.draw(); 
  glLineWidth(1.0);
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_faces() {
  shader_face_.use(); 
  update_face_uniforms();

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);
  glDepthFunc(GL_LESS);
  if (clipping_plane_.clipping_mode() ==
      internal::Clipping_plane::Clipping_mode::SOLID_HALF_TRANSPARENT_HALF) {
    // 1. draw solid first
    render_faces_bis(Rendering_mode::DRAW_INSIDE_ONLY);

    // 2. draw transparent layer second with back face culling
    glDepthMask(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    render_faces_bis(Rendering_mode::DRAW_OUTSIDE_ONLY);

    // 3. draw solid again without culling and blend
    glDepthMask(true);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    render_faces_bis(Rendering_mode::DRAW_INSIDE_ONLY);
  } else {
    if (clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_WIRE_HALF) ||
        clipping_plane_.clipping_mode_is(internal::Clipping_plane::Clipping_mode::SOLID_HALF_ONLY)) {
      render_faces_bis(Rendering_mode::DRAW_INSIDE_ONLY);
    } else {
      render_faces_bis(Rendering_mode::DRAW_ALL);
    }
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_faces_bis(Rendering_mode mode) {
  shader_face_.uniform("u_RenderingMode", static_cast<float>(mode));
  faces_.draw(); 
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_normals() {
   update_normals_uniforms();

  glDepthFunc(GL_LEQUAL);
  faces_.draw();
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_mesh_triangles() {
  update_triangles_uniforms();

  glDepthFunc(GL_LEQUAL);
  faces_.draw(); 
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_clipping_plane() {
  if (!is_opengl_4_3_) return; 
  
  gizmos_.render_clipping_plane_gizmo(
    clipping_plane_.model_matrix(), 
    view_projection_matrix_);
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_xy_grid() {
  gizmos_.render_xy_grid_gizmo(
    view_projection_matrix_, 
    camera_.distance());
} 

CGAL_INLINE_FUNCTION
void Scene_renderer::render_world_axis() {
  gizmos_.render_world_axis_gizmo(
    view_matrix_, 
    window_.aspect_ratio(), 
    window_.framebuffer_width(), 
    window_.framebuffer_height());
}

CGAL_INLINE_FUNCTION
vec3f Scene_renderer::color_to_normalized_vec3(const CGAL::IO::Color &c) {
  return {static_cast<float>(c.red()) / 255,
          static_cast<float>(c.green()) / 255,
          static_cast<float>(c.blue()) / 255};
}

CGAL_INLINE_FUNCTION
vec4f Scene_renderer::color_to_normalized_vec4(const CGAL::IO::Color &c) {
  return {static_cast<float>(c.red()) / 255,
          static_cast<float>(c.green()) / 255,
          static_cast<float>(c.blue()) / 255, 1.0f};
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_SCENE_RENDERER_IMPL_H
