#ifndef CGAL_GLFW_BASIC_VIEWER_IMPL_H
#define CGAL_GLFW_BASIC_VIEWER_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include <CGAL/GLFW/Basic_viewer.h>

#else
#define CGAL_INLINE_FUNCTION
#endif

#include <memory>

#include <CGAL/GLFW/bv_settings.h>
#include <CGAL/GLFW/internal/binding.h>
#include <CGAL/GLFW/internal/event.h>
#include <CGAL/GLFW/internal/Action_registry.h>
#include <CGAL/GLFW/internal/Animation_controller.h>

namespace CGAL {
namespace GLFW {

CGAL_INLINE_FUNCTION
Basic_viewer::Basic_viewer(const Graphics_scene &graphic_scene,
                           const char *title, 
                           const Basic_viewer_options& opts)
    : scene_(graphic_scene), 
      title_(title), 
      draw_vertices_(opts.init_draw_vertices),
      draw_edges_(opts.init_draw_edges), 
      draw_faces_(opts.init_draw_faces),
      draw_rays_(opts.init_draw_rays), 
      draw_lines_(opts.init_draw_lines),
      use_mono_color_(opts.init_use_mono_color),
      inverse_normal_(opts.init_inverse_normal), flat_shading_(opts.init_flat_shading) 
{
  setup_context(opts.hidden_window); 
  setup_inputs(); 
}

CGAL_INLINE_FUNCTION
Basic_viewer::~Basic_viewer() {
  world_axis_renderer_.delete_buffers();
  xy_grid_renderer_.delete_buffers();
  xy_axis_renderer_.delete_buffers();

  shader_face_.destroy();
  shader_sphere_.destroy();
  shader_line_.destroy();
  shader_pl_.destroy();
  shader_cylinder_.destroy();
  shader_plane_.destroy();
  shader_grid_.destroy();
  shader_normal_.destroy();
  shader_arrow_.destroy();
  shader_triangles_.destroy();

  clipping_plane_.reset(nullptr); 
  
  glDeleteBuffers(NB_GL_BUFFERS, vbo_);
  glDeleteVertexArrays(NB_VAO_BUFFERS, vao_);

  window_.reset(nullptr);
}

CGAL_INLINE_FUNCTION
bool Basic_viewer::setup_context(bool hidden) {
  const internal::Window_specification spec{
    .title = title_,
    .width = CGAL_WINDOW_WIDTH_INIT,
    .height = CGAL_WINDOW_HEIGHT_INIT,
    .gl_major = 4,
    .gl_minor = 3,
    .hidden = hidden 
  };

  window_ = std::make_unique<internal::Window>(spec);

  if (!window_->handle())
    return false;

  int opengl_major_version, opengl_minor_version;
  glGetIntegerv(GL_MAJOR_VERSION, &opengl_major_version);
  glGetIntegerv(GL_MINOR_VERSION, &opengl_minor_version);

  if (opengl_major_version > 4 ||
      opengl_major_version == 4 && opengl_minor_version >= 3) {
    is_opengl_4_3_ = true;
  }

  compile_shaders();
  initialize_camera();
  initialize_buffers();
  initialize_and_load_world_axis();
  initialize_and_load_clipping_plane();

  check_geometry_feature_availability();

  default_color_ray_ = color_to_normalized_vec3(scene_.get_default_color_ray());
  default_color_face_ =
      color_to_normalized_vec3(scene_.get_default_color_face());
  default_color_line_ =
      color_to_normalized_vec3(scene_.get_default_color_line());
  default_color_point_ =
      color_to_normalized_vec3(scene_.get_default_color_point());
  default_color_segment_ =
      color_to_normalized_vec3(scene_.get_default_color_segment());

  return true;
}

CGAL_INLINE_FUNCTION
bool Basic_viewer::setup_inputs() {
  action_registry_ = std::make_unique<internal::Action_registry>();
  animation_controller_ = std::make_unique<internal::Animation_controller>();

  register_actions();

  if (!window_)
    return false;

  auto callback = [this](const internal::Event& event) {
    internal::Event_context context{ .viewer = *this, .event = event };
    if (action_registry_->dispatch(context)) {
      need_update_ = true;
    } 
  };

  window_->on_key(callback);
  window_->on_mouse_btn(callback);
  window_->on_scroll(callback);

  window_->on_resize([this](const internal::Resize_event& event) {
    glViewport(0, 0, event.width, event.height);
    need_update_ = true;
  });

  window_->on_cursor_move([this](const internal::Cursor_event& e) {
    float dx = (e.xpos - last_x_) / window_->aspect_ratio();                                                
    float dy = (e.ypos - last_y_) / window_->aspect_ratio();                                                
    last_x_ = e.xpos;
    last_y_ = e.ypos;                                                                             
                                                                                                  
    auto mods = internal::Input::active_modifiers(window_->handle());
    action_registry_->for_each_hold<internal::Mouse_btn_binding>(                                 
      [&](const internal::Mouse_btn_binding& b) -> bool {                                         
        if (internal::Input::is_mouse_button_pressed(window_->handle(), b.button) && mods == b.mods) {
          internal::Drag_event ev{ b.button, b.mods, dx, dy };                  
          internal::Event_context ctx{ .viewer = *this, .event = ev };                            
          if (action_registry_->dispatch(ctx)) {
            need_update_ = true;             
          } 

          return true;                
        }
        return false;               
      });
  });

  action_registry_->print_help();

  return true;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::show() {
  // if (!setup_context(/*hidden=*/false) || !setup_inputs())
  //   return;
  if (!window_ || !window_->handle())
    return; 

  float elapsed_time = 0.0f;
  float last_frame = 0.0;
  while (!window_->should_close()) {
    float current_frame = static_cast<float>(glfwGetTime());
    delta_time_ = current_frame - last_frame;
    last_frame = current_frame;
    if (delta_time_ < 1e-3)
      delta_time_ = 1e-3;

    handle_events(delta_time_);
    if (need_update()) {
      render_scene(delta_time_);
      need_update_ = false;
    }
    print_application_state(elapsed_time, delta_time_);
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::make_screenshot(const std::string &filepath) {
  // if (!setup_context(/*hidden=*/true))
  //   return;

  camera_->disable_smoothness();

  GLuint fbo, color_rbo, depth_rbo;
  int width, height; 
  window_->window_size(width, height);

  // Create FBO
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // Color attachment
  glGenRenderbuffers(1, &color_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, color_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, color_rbo);

  // Depth attachment
  glGenRenderbuffers(1, &depth_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth_rbo);

  glViewport(0, 0, width, height);
  draw();

  // Read from FBO (GL_COLOR_ATTACHMENT0, not GL_FRONT/GL_BACK)
  capture_screenshot(filepath, true);

  // Cleanup
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteRenderbuffers(1, &color_rbo);
  glDeleteRenderbuffers(1, &depth_rbo);
  glDeleteFramebuffers(1, &fbo);

  std::cout << "Screenshot saved to " << filepath << "\n";
}

CGAL_INLINE_FUNCTION
void Basic_viewer::compile_shaders() {
  const char *PL_VERTEX =
      is_opengl_4_3_ ? VERTEX_SOURCE_P_L : VERTEX_SOURCE_P_L_COMP;
  const char *PL_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_pl_ = internal::Shader::create(PL_VERTEX, PL_FRAGMENT);
  if (!shader_pl_.is_valid()) {
    std::cerr << "Failed to create pl shader program\n";
  }

  const char *FACE_VERTEX =
      is_opengl_4_3_ ? VERTEX_SOURCE_COLOR : VERTEX_SOURCE_COLOR_COMP;
  const char *FACE_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_COLOR : FRAGMENT_SOURCE_COLOR_COMP;
  shader_face_ = internal::Shader::create(FACE_VERTEX, FACE_FRAGMENT);
  if (!shader_face_.is_valid()) {
    std::cerr << "Failed to create face shader program\n";
  }

  const char *PLANE_VERTEX = VERTEX_SOURCE_CLIPPING_PLANE;
  const char *PLANE_FRAGMENT = FRAGMENT_SOURCE_CLIPPING_PLANE;
  shader_plane_ = internal::Shader::create(PLANE_VERTEX, PLANE_FRAGMENT);
  if (!shader_plane_.is_valid()) {
    std::cerr << "Failed to create plane shader program\n";
  }

  const char *SHAPE_VERTEX = VERTEX_SOURCE_SHAPE;
  const char *POINT_GEOMETRY = GEOMETRY_SOURCE_SPHERE;
  shader_sphere_ = internal::Shader::create(SHAPE_VERTEX, PL_FRAGMENT, POINT_GEOMETRY);
  if (!shader_sphere_.is_valid()) {
    std::cerr << "Failed to create sphere shader program\n";
  }

  const char *EDGE_VERTEX = VERTEX_SOURCE_SHAPE;
  const char *EDGE_GEOMETRY = GEOMETRY_SOURCE_CYLINDER;
  const char *EDGE_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_cylinder_ = internal::Shader::create(EDGE_VERTEX, PL_FRAGMENT, EDGE_GEOMETRY);
  if (!shader_cylinder_.is_valid()) {
    std::cerr << "Failed to create cylinder shader program\n";
  }

  const char *LINE_VERTEX = VERTEX_SOURCE_LINE_WIDTH;
  const char *LINE_GEOMETRY = GEOMETRY_SOURCE_LINE_WIDTH;
  const char *LINE_FRAGMENT = FRAGMENT_SOURCE_P_L;
  shader_line_ = internal::Shader::create(LINE_VERTEX, LINE_FRAGMENT, LINE_GEOMETRY);
  if (!shader_line_.is_valid()) {
    std::cerr << "Failed to create line shader program\n";
  }

  const char *GRID_VERTEX = VERTEX_SOURCE_LINE;
  const char *GRID_GEOMETRY = GEOMETRY_SOURCE_LINE;
  const char *GRID_FRAGMENT = FRAGMENT_SOURCE_LINE;
  shader_grid_ = internal::Shader::create(GRID_VERTEX, GRID_FRAGMENT, GRID_GEOMETRY);
  if (!shader_grid_.is_valid()) {
    std::cerr << "Failed to create grid shader program\n";
  }

  const char *ARROW_VERTEX = VERTEX_SOURCE_LINE;
  const char *ARROW_GEOMETRY = GEOMETRY_SOURCE_ARROW;
  const char *ARROW_FRAGMENT = FRAGMENT_SOURCE_LINE;
  shader_arrow_ = internal::Shader::create(ARROW_VERTEX, ARROW_FRAGMENT, ARROW_GEOMETRY);
  if (!shader_arrow_.is_valid()) {
    std::cerr << "Failed to create arrow shader \n";
  }

  const char *NORMAL_VERTEX = VERTEX_SOURCE_NORMAL;
  const char *NORMAL_GEOMETRY = GEOMETRY_SOURCE_NORMAL;
  const char *NORMAL_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_normal_ =
      internal::Shader::create(NORMAL_VERTEX, NORMAL_FRAGMENT, NORMAL_GEOMETRY);
  if (!shader_normal_.is_valid()) {
    std::cerr << "Failed to create normal shader program\n";
  }

  const char *TRIANGLE_VERTEX = VERTEX_SOURCE_TRIANGLE;
  const char *TRIANGLE_GEOMETRY = GEOMETRY_SOURCE_TRIANGLE;
  const char *TRIANGLE_FRAGMENT =
      is_opengl_4_3_ ? FRAGMENT_SOURCE_P_L : FRAGMENT_SOURCE_P_L_COMP;
  shader_triangles_ =
      internal::Shader::create(TRIANGLE_VERTEX, TRIANGLE_FRAGMENT, TRIANGLE_GEOMETRY);
  if (!shader_triangles_.is_valid()) {
    std::cerr << "Failed to create triangles shader program\n";
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::initialize_camera() {
  vec3f pmin(scene_.bounding_box().xmin(), scene_.bounding_box().ymin(),
             scene_.bounding_box().zmin());

  vec3f pmax(scene_.bounding_box().xmax(), scene_.bounding_box().ymax(),
             scene_.bounding_box().zmax());

  bounding_box_ = {pmin, pmax};

  camera_ = std::make_unique<internal::Camera>(); 
  camera_->lookat(pmin, pmax);

  if (is_two_dimensional()) {
    camera_->set_constraint_axis(internal::Camera::Constraint_axis::FORWARD_AXIS);
    camera_->set_orthographic();
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::initialize_and_load_world_axis() {
  // World axis initialization
  world_axis_renderer_.initialize_buffers();
  world_axis_renderer_.set_width(3.f);
  world_axis_renderer_.add_line(vec3f::Zero(), .1f * vec3f::UnitX(),
                                vec3f(1, 0, 0)); // x-axis
  world_axis_renderer_.add_line(vec3f::Zero(), .1f * vec3f::UnitY(),
                                vec3f(0, 1, 0)); // y-axis
  world_axis_renderer_.add_line(vec3f::Zero(), .1f * vec3f::UnitZ(),
                                vec3f(0, 0, 1)); // z-axis
  world_axis_renderer_.load_buffers();

  float camera_size = camera_->get_size() * 0.5;
  // XY grid axis initialization
  xy_axis_renderer_.initialize_buffers();
  xy_axis_renderer_.set_width(5.f);
  xy_axis_renderer_.add_line(vec3f::Zero(), camera_size * vec3f::UnitX(),
                             vec3f(1, 0, 0)); // x-axis
  xy_axis_renderer_.add_line(vec3f::Zero(), camera_size * vec3f::UnitY(),
                             vec3f(0, 1, 0)); // y-axis
  xy_axis_renderer_.load_buffers();

  // XY grid initialization
  xy_grid_renderer_.initialize_buffers();
  xy_grid_renderer_.set_width(2.f);
  xy_grid_renderer_.add_line(vec3f::Zero(), -2.f * camera_size * vec3f::UnitX(),
                             vec3f(.8f, .8f, .8f)); // -x-axis
  xy_grid_renderer_.add_line(vec3f::Zero(), -2.f * camera_size * vec3f::UnitY(),
                             vec3f(.8f, .8f, .8f)); // -y-axis
  xy_grid_renderer_.add_line(vec3f::Zero(), -2.f * camera_size * vec3f::UnitZ(),
                             vec3f(.8f, .8f, .8f)); // -z-axis

  vec3f color(.8f, .8f, .8f);

  xy_grid_renderer_.generate_grid(color, camera_size);

  xy_grid_renderer_.load_buffers();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::load_buffer(int i, int location,
                               const std::vector<float> &vector,
                               int data_count) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_[i]);

  glBufferData(GL_ARRAY_BUFFER, vector.size() * sizeof(float), vector.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(location, data_count, GL_FLOAT, GL_FALSE,
                        data_count * sizeof(float), nullptr);

  glEnableVertexAttribArray(location);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::load_buffer(int i, int location, int gs_enum,
                               int data_count) {
  const auto &vector = scene_.get_array_of_index(gs_enum);
  load_buffer(i, location, vector, data_count);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::initialize_buffers() {
  glGenBuffers(NB_GL_BUFFERS, vbo_);
  glGenVertexArrays(NB_VAO_BUFFERS, vao_);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::load_scene() {
  unsigned int bufn = 0;

  std::vector<float> positions, normals, colors;

  // 1) POINT SHADER

  glBindVertexArray(vao_[VAO_POINTS]);
  positions = scene_.get_array_of_index(Graphics_scene::POS_POINTS);
  colors = scene_.get_array_of_index(Graphics_scene::COLOR_POINTS);

  load_buffer(bufn++, 0, positions, 3);
  load_buffer(bufn++, 1, colors, 3);

  // 2) SEGMENT SHADER

  glBindVertexArray(vao_[VAO_SEGMENTS]);
  positions = scene_.get_array_of_index(Graphics_scene::POS_SEGMENTS);
  colors = scene_.get_array_of_index(Graphics_scene::COLOR_SEGMENTS);

  load_buffer(bufn++, 0, positions, 3);
  load_buffer(bufn++, 1, colors, 3);

  // 3) RAYS SHADER

  glBindVertexArray(vao_[VAO_RAYS]);
  positions = scene_.get_array_of_index(Graphics_scene::POS_RAYS);
  colors = scene_.get_array_of_index(Graphics_scene::COLOR_RAYS);

  load_buffer(bufn++, 0, positions, 3);
  load_buffer(bufn++, 1, colors, 3);

  // 4) LINES SHADER

  glBindVertexArray(vao_[VAO_LINES]);
  positions = scene_.get_array_of_index(Graphics_scene::POS_LINES);
  colors = scene_.get_array_of_index(Graphics_scene::COLOR_LINES);

  load_buffer(bufn++, 0, positions, 3);
  load_buffer(bufn++, 1, colors, 3);

  // 5) FACE SHADER

  glBindVertexArray(vao_[VAO_FACES]);
  positions = scene_.get_array_of_index(Graphics_scene::POS_FACES);
  normals = scene_.get_array_of_index(
      flat_shading_ ? Graphics_scene::FLAT_NORMAL_FACES
                    : Graphics_scene::SMOOTH_NORMAL_FACES);
  colors = scene_.get_array_of_index(Graphics_scene::COLOR_FACES);

  load_buffer(bufn++, 0, positions, 3);
  load_buffer(bufn++, 1, normals, 3);
  load_buffer(bufn++, 2, colors, 3);

  are_buffers_initialized_ = true;
}

CGAL_INLINE_FUNCTION
CGAL::Plane_3<Basic_viewer::Local_kernel> Basic_viewer::clipping_plane() const {
  mat4f cpm = clipping_plane_->get_matrix();
  CGAL::Aff_transformation_3<Basic_viewer::Local_kernel> aff(
      cpm(0, 0), cpm(0, 1), cpm(0, 2), cpm(0, 3), cpm(1, 0), cpm(1, 1),
      cpm(1, 2), cpm(1, 3), cpm(2, 0), cpm(2, 1), cpm(2, 2), cpm(2, 3));

  CGAL::Plane_3<Local_kernel> p3(0, 0, 1, 0);
  return p3.transform(aff);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::compute_model_view_projection_matrix(const float delta_time) {
  camera_->update(delta_time);
  clipping_plane_->update(delta_time);

  if (animation_controller_->is_running()) {
    internal::Animation_key_frame animation_frame = animation_controller_->run();
    camera_->set_orientation(animation_frame.orientation);
    camera_->set_position(animation_frame.position);
  }

  view_matrix_ = camera_->view();
  projection_matrix_ = camera_->projection(window_->framebuffer_width(), window_->framebuffer_height());

  view_projection_matrix_ = projection_matrix_ * view_matrix_;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_face_uniforms() {
  if (shader_face_.is_valid()) {
    shader_face_.use();

    shader_face_.set_mat4f("u_Mvp", view_projection_matrix_.data());
    shader_face_.set_mat4f("u_Mv", view_matrix_.data());
    if (use_mono_color_) {
      shader_face_.set_int("u_UseDefaultColor", 1);
      shader_face_.set_vec3f("u_DefaultColor", default_color_face_.data());
    } else {
      shader_face_.set_int("u_UseDefaultColor", 0);
    }

    shader_face_.set_vec4f("u_LightPos", light_position_.data());
    shader_face_.set_vec4f("u_LightDiff", diffuse_color_.data());
    shader_face_.set_vec4f("u_LightSpec", specular_color_.data());
    shader_face_.set_vec4f("u_LightAmb", ambient_color_.data());
    shader_face_.set_float("u_SpecPower", shininess_);

    shader_face_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_face_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_face_.set_float("u_RenderingTransparency",
                           clipping_plane_->get_transparency());
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_sphere_uniforms() {
  if (shader_sphere_.is_valid()) {
    shader_sphere_.use();

    bool half = clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    if (use_mono_color_) {
      shader_sphere_.set_int("u_UseDefaultColor", 1);
      shader_sphere_.set_vec3f("u_DefaultColor", default_color_point_.data());
    } else {
      shader_sphere_.set_int("u_UseDefaultColor", 0);
    }

    shader_sphere_.set_float("u_RenderingMode", static_cast<float>(mode));

    shader_sphere_.set_mat4f("u_Mvp", view_projection_matrix_.data());
    shader_sphere_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_sphere_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_sphere_.set_float("u_Radius",
                             camera_->get_radius() * size_vertices_ * 0.001);
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_cylinder_uniforms() {
  if (shader_cylinder_.is_valid()) {
    shader_cylinder_.use();

    bool half = clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    if (use_mono_color_) {
      shader_cylinder_.set_int("u_UseDefaultColor", 1);
      shader_cylinder_.set_vec3f("u_DefaultColor",
                                 default_color_segment_.data());
    } else {
      shader_cylinder_.set_int("u_UseDefaultColor", 0);
    }

    shader_cylinder_.set_float("u_RenderingMode", static_cast<float>(mode));

    shader_cylinder_.set_mat4f("u_Mvp", view_projection_matrix_.data());
    shader_cylinder_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_cylinder_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_cylinder_.set_float("u_Radius",
                               camera_->get_radius() * size_edges_ * 0.001);
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_pl_uniforms(const vec3f &default_color) {
  if (shader_pl_.is_valid()) {
    shader_pl_.use();

    bool half = clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    shader_pl_.set_mat4f("u_Mvp", view_projection_matrix_.data());
    shader_pl_.set_float("u_PointSize",
                         camera_->get_radius() * size_vertices_ * 0.1);
    shader_pl_.set_int("u_IsOrthographic",
                       static_cast<int>(camera_->is_orthographic()));
    if (use_mono_color_) {
      shader_pl_.set_int("u_UseDefaultColor", 1);
      shader_pl_.set_vec3f("u_DefaultColor", default_color.data());
    } else {
      shader_pl_.set_int("u_UseDefaultColor", 0);
    }

    shader_pl_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_pl_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_pl_.set_float("u_RenderingMode", static_cast<float>(mode));
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_line_uniforms(float size,
                                        const vec3f &default_color) {
  if (shader_line_.is_valid()) {
    shader_line_.use();

    bool half = clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    float viewport[2] = {static_cast<float>(window_->framebuffer_width()),
                         static_cast<float>(window_->framebuffer_height())};

    shader_line_.set_mat4f("u_Mvp", view_projection_matrix_.data());
    shader_line_.set_float("u_PointSize", size);
    shader_line_.set_int("u_IsOrthographic",
                         static_cast<int>(camera_->is_orthographic()));
    if (use_mono_color_) {
      shader_line_.set_int("u_UseDefaultColor", 1);
      shader_line_.set_vec3f("u_DefaultColor", default_color.data());
    } else {
      shader_line_.set_int("u_UseDefaultColor", 0);
    }

    shader_line_.set_vec2f("u_Viewport", &viewport[0]);

    shader_line_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_line_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_line_.set_float("u_RenderingMode", static_cast<float>(mode));
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_clipping_uniforms() {
  mat4f clipping_model_matrix = clipping_plane_->get_matrix();

  point_plane_ = clipping_model_matrix * vec4f(0, 0, 0, 1);
  clip_plane_ = clipping_model_matrix * vec4f(0, 0, 1, 0);

  if (shader_plane_.is_valid()) {
    shader_plane_.use();
    shader_plane_.set_mat4f("u_Vp", view_projection_matrix_.data());
    shader_plane_.set_mat4f("u_M", clipping_model_matrix.data());
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_world_axis_uniforms() {
  mat4f view = view_matrix_;

  // we only want the rotation part of the view matrix
  mat3f rotation = view.block<3, 3>(0, 0);

  mat4f rotation_4x4 = mat4f::Identity();
  rotation_4x4.block<3, 3>(0, 0) = rotation;

  float half_width = window_->aspect_ratio() * 0.1f;
  float half_height = 0.1f;
  mat4f projection = internal::utils::ortho(-half_width, half_width, -half_height,
                                  half_height, -1.0f, 1.0f);

  mat4f translation = internal::transform::translation(
      vec3f(half_width - 0.1f * window_->aspect_ratio(), half_height - 0.1f, 0.0f));

  mat4f mvp = projection * rotation_4x4 * translation;

  if (shader_arrow_.is_valid()) {
    shader_arrow_.use();
    shader_arrow_.set_mat4f("u_Mvp", mvp.data());
    shader_arrow_.set_float("u_SceneRadius", 1.0f);
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_xy_axis_uniforms() {
  if (shader_arrow_.is_valid()) {
    shader_arrow_.use();
    shader_arrow_.set_mat4f("u_Mvp", view_projection_matrix_.data());
    shader_arrow_.set_float("u_SceneRadius", camera_->get_radius());
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_xy_grid_uniforms() {
  if (shader_grid_.is_valid()) {
    shader_grid_.use();
    shader_grid_.set_mat4f("u_Mvp", view_projection_matrix_.data());
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_normals_uniforms() {
  if (shader_normal_.is_valid()) {
    shader_normal_.use();

    bool half = clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    vec4f color = color_to_normalized_vec4(default_color_normal_);
    shader_normal_.set_mat4f("u_Mv", view_matrix_.data());
    if (use_mono_color_normal_) {
      shader_normal_.set_int("u_UseDefaultColor", 1);
      shader_normal_.set_vec3f("u_DefaultColor", color.data());
    } else {
      shader_normal_.set_int("u_UseDefaultColor", 0);
    }
    shader_normal_.set_mat4f("u_Projection", projection_matrix_.data());
    shader_normal_.set_float("u_Factor", normal_height_factor_);
    shader_normal_.set_float("u_SceneRadius", camera_->get_radius());
    if (display_face_normal_) {
      shader_normal_.set_int("u_DisplayFaceNormal", 1);
    } else {
      shader_normal_.set_int("u_DisplayFaceNormal", 0);
    }

    shader_normal_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_normal_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_normal_.set_float("u_RenderingMode", static_cast<float>(mode));
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::update_triangles_uniforms() {
  if (shader_triangles_.is_valid()) {
    shader_triangles_.use();

    bool half = clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY);
    auto mode =
        half ? Rendering_mode::DRAW_INSIDE_ONLY : Rendering_mode::DRAW_ALL;

    shader_triangles_.set_mat4f("u_Mvp", view_projection_matrix_.data());

    shader_triangles_.set_vec4f("u_ClipPlane", clip_plane_.data());
    shader_triangles_.set_vec4f("u_PointPlane", point_plane_.data());
    shader_triangles_.set_float("u_RenderingMode", static_cast<float>(mode));
  }
}

CGAL_INLINE_FUNCTION
bool Basic_viewer::need_update() const {
  return camera_->need_update() 
      || clipping_plane_->need_update() 
      || animation_controller_->is_running() 
      || need_update_;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::handle_events(float dt) {
  glfwPollEvents();

  internal::Modifier mods = internal::Input::active_modifiers(window_->handle()); 
  action_registry_->for_each_hold<internal::Key_binding>([&](const internal::Key_binding& b) {
    if (internal::Input::is_key_pressed(window_->handle(), b.key) && mods == b.mods) {
      internal::Key_event event {.key = b.key, .scancode = 0, .action = b.action, .mods = b.mods }; 
      internal::Event_context context { .viewer = *this, .event = event, .dt = dt};
      if (action_registry_->dispatch(context)) need_update_ = true; 
      return false; // don't stop at the first key hold trigger 
    } 
    
    return true; 
  }); 
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_scene(const float delta_time) {
  draw(delta_time);
  window_->update();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::draw(const float delta_time) {
  if (!are_buffers_initialized_) {
    load_scene();
  }

  glClearColor(1.0f, 1.0f, 1.0f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_LINE_SMOOTH);

  compute_model_view_projection_matrix(delta_time);

  if (draw_rays_) {
    render_rays();
  }

  if (draw_lines_) {
    render_lines();
  }

  if (draw_edges_ && !draw_mesh_triangles_) {
    render_edges();
  }

  if (draw_vertices_) {
    render_vertices();
  }

  if (clipping_plane_enabled()) {
    render_clipping_plane();
  }

  if (draw_normals_) {
    render_normals();
  }

  if (draw_faces_) {
    render_faces();
  }

  if (draw_mesh_triangles_) {
    render_triangles();
  }

  if (draw_world_axis_) {
    render_world_axis();
  }

  if (draw_xy_grid_) {
    render_xy_grid();
  }
}

CGAL_INLINE_FUNCTION
vec4f Basic_viewer::color_to_normalized_vec4(const CGAL::IO::Color &c) const {
  return {static_cast<float>(c.red()) / 255,
          static_cast<float>(c.green()) / 255,
          static_cast<float>(c.blue()) / 255, 1.0f};
}

CGAL_INLINE_FUNCTION
vec3f Basic_viewer::color_to_normalized_vec3(const CGAL::IO::Color &c) const {
  return {static_cast<float>(c.red()) / 255,
          static_cast<float>(c.green()) / 255,
          static_cast<float>(c.blue()) / 255};
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_faces() {
  update_face_uniforms();

  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(2.0, 2.0);
  glDepthFunc(GL_LESS);
  if (clipping_plane_->display_mode() ==
      internal::Clipping_plane::Display_mode::SOLID_HALF_TRANSPARENT_HALF) {
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
    if (clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_WIRE_HALF) ||
        clipping_plane_->display_mode_is(internal::Clipping_plane::Display_mode::SOLID_HALF_ONLY)) {
      render_faces_bis(Rendering_mode::DRAW_INSIDE_ONLY);
    } else {
      render_faces_bis(Rendering_mode::DRAW_ALL);
    }
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_faces_bis(Rendering_mode mode) {
  shader_face_.set_float("u_RenderingMode", static_cast<float>(mode));

  glBindVertexArray(vao_[VAO_FACES]);
  glDrawArrays(GL_TRIANGLES, 0,
               scene_.number_of_elements(Graphics_scene::POS_FACES));
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_rays() {
  update_pl_uniforms(default_color_ray_);

  shader_pl_.set_float("u_RenderingMode",
                       static_cast<float>(Rendering_mode::DRAW_ALL));

  glLineWidth(size_rays_);
  glBindVertexArray(vao_[VAO_RAYS]);
  glDrawArrays(GL_LINES, 0,
               scene_.number_of_elements(Graphics_scene::POS_RAYS));
  glLineWidth(1.0);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_vertices() {
  update_pl_uniforms(default_color_point_);
  if (draw_sphere_vertex_ && geometry_feature_enabled_) {
    update_sphere_uniforms();
  }

  glDepthFunc(GL_LEQUAL);
  glBindVertexArray(vao_[VAO_POINTS]);
  glDrawArrays(GL_POINTS, 0,
               scene_.number_of_elements(Graphics_scene::POS_POINTS));
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_lines() {
  update_pl_uniforms(default_color_line_);

  shader_pl_.set_float("u_RenderingMode",
                       static_cast<float>(Rendering_mode::DRAW_ALL));

  glLineWidth(size_lines_);
  glBindVertexArray(vao_[VAO_LINES]);
  glDrawArrays(GL_LINES, 0,
               scene_.number_of_elements(Graphics_scene::POS_LINES));
  glLineWidth(1.0);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_edges() {
  update_line_uniforms(size_edges_, default_color_segment_);
  if (draw_cylinder_edge_ && geometry_feature_enabled_) {
    update_cylinder_uniforms();
  }

  glDepthFunc(GL_LEQUAL);
  glBindVertexArray(vao_[VAO_SEGMENTS]);
  glDrawArrays(GL_LINES, 0,
               scene_.number_of_elements(Graphics_scene::POS_SEGMENTS));
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_world_axis() {
  update_world_axis_uniforms();
  glDepthFunc(GL_LEQUAL);

  int w = window_->framebuffer_width();
  int h = window_->framebuffer_height();

  // Draw on the top-right corner of the screen
  glViewport(w - w / 5, h - h / 5, w / 5, h / 5);
  world_axis_renderer_.draw();

  // Restore the main viewport
  glViewport(0, 0, w, h);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_xy_grid() {
  glDepthFunc(GL_LEQUAL);

  update_xy_grid_uniforms();
  xy_grid_renderer_.draw();
  update_xy_axis_uniforms();
  xy_axis_renderer_.draw();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_normals() {
  update_normals_uniforms();

  glDepthFunc(GL_LEQUAL);
  glBindVertexArray(vao_[VAO_FACES]);
  glLineWidth(size_normals_);
  glDrawArrays(GL_TRIANGLES, 0,
               scene_.number_of_elements(Graphics_scene::POS_FACES));
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_triangles() {
  update_triangles_uniforms();

  glDepthFunc(GL_LEQUAL);
  glBindVertexArray(vao_[VAO_FACES]);
  glDrawArrays(GL_TRIANGLES, 0,
               scene_.number_of_elements(Graphics_scene::POS_FACES));
}

CGAL_INLINE_FUNCTION
void Basic_viewer::initialize_and_load_clipping_plane() {
  float size = ((scene_.bounding_box().xmax() - scene_.bounding_box().xmin()) +
                (scene_.bounding_box().ymax() - scene_.bounding_box().ymin()) +
                (scene_.bounding_box().zmax() - scene_.bounding_box().zmin()));

  clipping_plane_ = std::make_unique<internal::Clipping_plane>(size); 
  clipping_plane_->set_size(camera_->get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::render_clipping_plane() {
  if (!is_opengl_4_3_) {
    return;
  }

  update_clipping_uniforms();
  clipping_plane_->render();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::print_application_state(float &elapsed_time, const float delta_time) {
  elapsed_time += delta_time;
  if (elapsed_time * 1000 > 100) // update terminal display each 100ms
  {
    elapsed_time = 0.0f;
    if (print_application_state_) {
      std::cout
          << "\33[2K" << std::round(1 / delta_time) << " fps    "
          << delta_time * 1000 << " ms\n\33[2K"
          << "Camera type: " << (camera_->is_orbiter() ? "ORBITER" : "FREE_FLY")
          << "    "
          << "Camera mode: "
          << (camera_->is_orthographic() ? "ORTHOGRAPHIC" : "PERSPECTIVE")
          << "    "
          << "FOV: " << camera_->get_fov() << " \n\33[2K"
          << "Camera translation speed: " << camera_->get_translation_speed()
          << "    "
          << "Camera rotation speed: "
          << std::round(camera_->get_rotation_speed()) << "    "
          << "Camera constraint axis: " << camera_->get_constraint_axis_str()
          << "\n\33[2K"
          << "CP translation speed: " << clipping_plane_->get_translation_speed()
          << "    "
          << "CP rotation speed: "
          << std::round(clipping_plane_->get_rotation_speed()) << "    "
          << "CP constraint axis: " << clipping_plane_->get_constraint_axis_str()
          << "\n\33[2K"
          << "Reversed normals : " << (inverse_normal_ ? "TRUE" : "FALSE")
          << "    "
          << "Face normal : " << (display_face_normal_ ? "TRUE" : "FALSE")
          << "    "
          << "Shading : " << (flat_shading_ ? "FLAT" : "SMOOTH") << "\n\33[2K"
          << "Draw faces: " << (draw_faces_ ? "TRUE" : "FALSE") << "    "
          << "Draw edges: " << (draw_edges_ ? "TRUE" : "FALSE") << "    "
          << "Draw vertices: " << (draw_vertices_ ? "TRUE" : "FALSE")
          << "\n\33[2K"
          << "Draw lines: " << (draw_lines_ ? "TRUE" : "FALSE") << "    "
          << "Draw rays: " << (draw_rays_ ? "TRUE" : "FALSE") << "    "
          << "Draw normals: " << (draw_normals_ ? "TRUE" : "FALSE")
          << "\n\33[2K"
          << "Draw edges as cylinder: "
          << (draw_cylinder_edge_ ? "TRUE" : "FALSE") << "    "
          << "Draw vertices as sphere: "
          << (draw_sphere_vertex_ ? "TRUE" : "FALSE") << "\n\33[2K"
          << "Use default color: " << (use_mono_color_ ? "TRUE" : "FALSE")
          << "    "
          << "Number of key frames: "
          << animation_controller_->number_of_key_frames() << "\n\33[2K"
          << "Light color: (" << ambient_color_.x() << ", "
          << ambient_color_.y() << ", " << ambient_color_.z() << ")\n\33[2K"
          << "Size of vertices: " << size_vertices_
          << "    Size of edges: " << size_edges_ << "    "
          << "\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\033[F\r"
          << std::flush;
    }
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::check_geometry_feature_availability() {
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
void Basic_viewer::change_pivot_point() {
  auto [mouse_x, mouse_y] = internal::Input::mouse_position(window_->handle());
  int width, height;  
  window_->window_size(width, height);

  vec2f nc = internal::utils::normalized_coordinates({mouse_x, mouse_y}, width, height);

  vec3f camera_position = camera_->get_position();
  float camera_x = camera_position.x();
  float camera_y = camera_position.y();

  float x_value = nc.x() + camera_x;
  float y_value = nc.y() + camera_y;

  if (internal::utils::inside_bounding_box_2d(
          {x_value, y_value},
          {bounding_box_.first.x(), bounding_box_.first.y()},
          {bounding_box_.second.x(), bounding_box_.second.y()})) {
    std::cout << "INSIDE\n";
    camera_->set_center({nc.x(), nc.y(), 0});
  } else {
    camera_->set_center(
        internal::utils::center(bounding_box_.first, bounding_box_.second));
    std::cout << "OUTSIDE\n";
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::increase_size_edge(const float delta_time) {
  float upper_bound = 20.0;
  size_edges_ = std::min(upper_bound, size_edges_ + 10.0f * delta_time);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::decrease_size_edge(const float delta_time) {
  float lower_bound = 0.01;
  size_edges_ = std::max(lower_bound, size_edges_ - 10.0f * delta_time);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::increase_size_vertex(const float delta_time) {
  float upper_bound = 50.0;
  size_vertices_ = std::min(upper_bound, size_vertices_ + 10.0f * delta_time);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::decrease_size_vertex(const float delta_time) {
  float lower_bound = 0.01;
  size_vertices_ = std::max(lower_bound, size_vertices_ - 10.0f * delta_time);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::increase_red_component(const float delta_time) {
  float speed = delta_time;
  ambient_color_.x() += speed;
  if (ambient_color_.x() > 1.f)
    ambient_color_.x() = 1.f;
  if (ambient_color_.x() < 0.f)
    ambient_color_.x() = 0.f;

  diffuse_color_.x() += speed;
  if (diffuse_color_.x() > 1.f)
    diffuse_color_.x() = 1.f;
  if (diffuse_color_.x() < 0.f)
    diffuse_color_.x() = 0.f;

  specular_color_.x() += speed;
  if (specular_color_.x() > 1.f)
    specular_color_.x() = 1.f;
  if (specular_color_.x() < 0.f)
    specular_color_.x() = 0.f;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::increase_green_component(const float delta_time) {
  float speed = delta_time;
  ambient_color_.y() += speed;
  if (ambient_color_.y() > 1.f)
    ambient_color_.y() = 1.f;
  if (ambient_color_.y() < 0.f)
    ambient_color_.y() = 0.f;

  diffuse_color_.y() += speed;
  if (diffuse_color_.y() > 1.f)
    diffuse_color_.y() = 1.f;
  if (diffuse_color_.y() < 0.f)
    diffuse_color_.y() = 0.f;

  specular_color_.y() += speed;
  if (specular_color_.y() > 1.f)
    specular_color_.y() = 1.f;
  if (specular_color_.y() < 0.f)
    specular_color_.y() = 0.f;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::increase_blue_component(const float delta_time) {
  float speed = delta_time;
  ambient_color_.z() += speed;
  if (ambient_color_.z() > 1.f)
    ambient_color_.z() = 1.f;
  if (ambient_color_.z() < 0.f)
    ambient_color_.z() = 0.f;

  diffuse_color_.z() += speed;
  if (diffuse_color_.z() > 1.f)
    diffuse_color_.z() = 1.f;
  if (diffuse_color_.z() < 0.f)
    diffuse_color_.z() = 0.f;

  specular_color_.z() += speed;
  if (specular_color_.z() > 1.f)
    specular_color_.z() = 1.f;
  if (specular_color_.z() < 0.f)
    specular_color_.z() = 0.f;
}

CGAL_INLINE_FUNCTION
void Basic_viewer::increase_light_all(const float delta_time) {
  increase_red_component(delta_time);
  increase_green_component(delta_time);
  increase_blue_component(delta_time);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::capture_screenshot(const std::string &filepath,
                                      bool use_framebuffer) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);

  const GLsizei NB_CHANNELS = 4;
  GLsizei stride = NB_CHANNELS * window_->framebuffer_width();
  stride += (stride % 4) ? (4 - stride % 4) : 0;
  GLsizei buffer_size = stride * window_->framebuffer_height();

  std::vector<char> buffer(buffer_size);
  if (shader_face_.is_valid()) {
    shader_face_.use();
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    glReadBuffer(use_framebuffer ? GL_COLOR_ATTACHMENT0 : GL_FRONT);
    glReadPixels(0, 0, window_->framebuffer_width(), window_->framebuffer_height(), 
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

    stbi_flip_vertically_on_write(true);
    if (!stbi_write_png(filepath.data(), window_->framebuffer_width(), window_->framebuffer_height(), 
                        NB_CHANNELS, buffer.data(), stride)) {
      std::cerr << "Failed to write screenshot to " << filepath << '\n';
    }
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::zoom_camera(float scroll_y) {
  float yoffset = scroll_y / window_->aspect_ratio();
  camera_->move(8.f * yoffset * CGAL_CAMERA_ZOOM_SPEED);
  clipping_plane_->set_size(camera_->get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::change_camera_fov(float scroll_y) {
  if (camera_->is_orthographic()) return;
  float yoffset = scroll_y / window_->aspect_ratio();
  camera_->increase_fov(yoffset);
  clipping_plane_->set_size(camera_->get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::reset_camera() {
  camera_->reset_position();
  camera_->reset_orientation();
}

CGAL_INLINE_FUNCTION
void Basic_viewer::reset_camera_and_clipping_plane() {
  camera_->reset_all();
  clipping_plane_->reset_all();
  clipping_plane_->set_size(camera_->get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::rotate_clipping_plane(float dx, float dy) {
  if (!clipping_plane_->display_mode_enabled()) return;
  clipping_plane_->set_right_axis(camera_->get_right());
  clipping_plane_->set_up_axis(camera_->get_up());
  clipping_plane_->rotation(dx, dy);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::translate_clipping_plane(float dx, float dy) {
  if (!clipping_plane_->display_mode_enabled()) return;
  clipping_plane_->translation(
    -dx * CGAL_CLIPPING_PLANE_DRAG_TRANSLATION_SPEED,
     dy * CGAL_CLIPPING_PLANE_DRAG_TRANSLATION_SPEED);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::translate_clipping_plane_along_camera(float dx, float dy) {
  // Use whichever axis the user dragged most along, with the y-axis taking
  // precedence and inverted so that dragging up pushes the plane forward.
  float s = (std::fabs(dy) > std::fabs(dx)) ? -dy : dx;
  clipping_plane_->translation(camera_->get_forward(),
                               s * CGAL_CLIPPING_PLANE_DRAG_TRANSLATION_SPEED);
}

CGAL_INLINE_FUNCTION
void Basic_viewer::reset_clipping_plane() {
  clipping_plane_->reset_all();
  clipping_plane_->set_size(camera_->get_size());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::save_key_frame() {
  if (!camera_->is_orbiter()) return;
  animation_controller_->add_key_frame(camera_->get_position(),
                                       camera_->get_orientation());
}

CGAL_INLINE_FUNCTION
void Basic_viewer::run_or_stop_animation() {
  if (!camera_->is_orbiter()) return;
  if (animation_controller_->is_running()) {
    animation_controller_->stop(animation_controller_->get_frame());
  } else {
    animation_controller_->start();
  }
}

CGAL_INLINE_FUNCTION
void Basic_viewer::register_actions() const {
  register_clipping_plane_actions();
  register_application_actions();
  register_camera_actions();
  register_window_actions();
  register_scene_actions();
  register_light_actions();
  register_animation_actions();
}

} // namespace GLFW
} // namespace CGAL

#include <CGAL/GLFW/internal/Basic_viewer_actions.inc.h>

#endif // CGAL_GLFW_BASIC_VIEWER_IMPL_H
