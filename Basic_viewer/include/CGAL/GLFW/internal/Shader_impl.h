#ifndef CGAL_GLFW_INTERNAL_SHADER_IMPL_H
#define CGAL_GLFW_INTERNAL_SHADER_IMPL_H

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Shader.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <iostream>

#include <glad/gl.h>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Shader::Shader(const GLuint program_id) : program_id_(program_id) {}

CGAL_INLINE_FUNCTION
void Shader::use() const { glUseProgram(program_id_); }

CGAL_INLINE_FUNCTION
void Shader::destroy() { glDeleteProgram(program_id_); }

CGAL_INLINE_FUNCTION
bool Shader::is_valid() const { return program_id_ != 0; }

CGAL_INLINE_FUNCTION
GLint Shader::get_uniform_location(const GLchar *name) {
  GLint loc = cached_uniforms_[name];
  if (loc != 0) {
    return loc;
  }

  loc = glGetUniformLocation(program_id_, name);
  cached_uniforms_[name] = loc;
  return loc;
}

CGAL_INLINE_FUNCTION
void Shader::set_mat4f(const GLchar *name, const GLfloat *data,
                       GLboolean transpose) {
  glUniformMatrix4fv(get_uniform_location(name), 1, transpose, data);
}

CGAL_INLINE_FUNCTION
void Shader::set_vec4f(const GLchar *name, const GLfloat *data) {
  glUniform4fv(get_uniform_location(name), 1, data);
}

CGAL_INLINE_FUNCTION
void Shader::set_vec3f(const GLchar *name, const GLfloat *data) {
  glUniform3fv(get_uniform_location(name), 1, data);
}

CGAL_INLINE_FUNCTION
void Shader::set_vec2f(const GLchar *name, const GLfloat *data) {
  glUniform2fv(get_uniform_location(name), 1, data);
}

CGAL_INLINE_FUNCTION
void Shader::set_float(const GLchar *name, const float data) {
  glUniform1f(get_uniform_location(name), data);
}

CGAL_INLINE_FUNCTION
void Shader::set_int(const GLchar *name, const int data) {
  glUniform1i(get_uniform_location(name), data);
}

CGAL_INLINE_FUNCTION
GLchar *Shader::enum_type_to_str(const GLenum type) {
  switch (type) {
  case GL_VERTEX_SHADER:
    return "VERTEX";
  case GL_GEOMETRY_SHADER:
    return "GEOMETRY";
  case GL_FRAGMENT_SHADER:
    return "FRAGMENT";
  default:
    std::cerr << "Unsupported shader enum type!" << std::endl;
    return "UNSUPPORTED TYPE";
  }
}

CGAL_INLINE_FUNCTION
GLuint Shader::compile_shader(const GLenum type, const GLchar *source) {
  GLuint shader_id = glCreateShader(type);
  glShaderSource(shader_id, 1, &source, nullptr);
  glCompileShader(shader_id);
  Shader::check_compile_errors(shader_id, Shader::enum_type_to_str(type));
  return shader_id;
}

CGAL_INLINE_FUNCTION
Shader Shader::create(const GLchar *source_vertex,
                      const GLchar *source_fragment,
                      const GLchar *source_geometry) {
  // Compile VERTEX shader
  GLuint vertex_shader_id =
      Shader::compile_shader(GL_VERTEX_SHADER, source_vertex);

  // Compile GEOMETRY shader if exists
  GLuint geometry_shader_id;
  if (source_geometry) {
    geometry_shader_id =
        Shader::compile_shader(GL_GEOMETRY_SHADER, source_geometry);
  }

  // Compile FRAGMENT shader
  GLuint fragment_shader_id =
      Shader::compile_shader(GL_FRAGMENT_SHADER, source_fragment);

  // Create a program object and link the compiled shaders to it
  GLuint program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  if (source_geometry) {
    glAttachShader(program_id, geometry_shader_id);
  }
  glAttachShader(program_id, fragment_shader_id);

  glLinkProgram(program_id);
  if (!Shader::check_compile_errors(program_id, "PROGRAM")) {
    program_id =
        0; // invalidate the program if the compilation or linkage fails
  }

  glDeleteShader(vertex_shader_id);
  if (source_geometry) {
    glDeleteShader(geometry_shader_id);
  }
  glDeleteShader(fragment_shader_id);

  return Shader(program_id);
}

CGAL_INLINE_FUNCTION
int Shader::check_compile_errors(const GLuint shader_id, const GLchar *type) {
  GLint success;
  GLchar info_log[1024];

  if (type != "PROGRAM") {
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

    if (!success) {
      glGetShaderInfoLog(shader_id, 1024, NULL, info_log);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
          << info_log
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
      return 0;
    }
  } else {
    glGetProgramiv(shader_id, GL_LINK_STATUS, &success);

    if (!success) {
      glGetProgramInfoLog(shader_id, 1024, NULL, info_log);
      std::cout
          << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
          << info_log
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
      return 0;
    }
  }

  return 1; // successful compilation/linkage
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_SHADER_IMPL_H
