#ifndef CGAL_GLFW_INTERNAL_SHADER_PROGRAM_IMPL_H
#define CGAL_GLFW_INTERNAL_SHADER_PROGRAM_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Shader_program.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <iostream>

#include <glad/gl.h>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
Shader_program::Shader_program(Shader_program&& other) noexcept : 
  cached_uniforms_(std::move(other.cached_uniforms_)), program_id_(other.program_id_) {
    other.program_id_ = 0; 
} 

CGAL_INLINE_FUNCTION
Shader_program& Shader_program::operator=(Shader_program&& other) noexcept {
  if (&other != this) {
    destroy(); 
    cached_uniforms_ = std::move(other.cached_uniforms_);
    program_id_ = other.program_id_; 
    other.program_id_ = 0; 
  }

  return *this; 
} 

CGAL_INLINE_FUNCTION
void Shader_program::destroy() { 
  if (program_id_ != 0) {
    glDeleteProgram(program_id_); 
    program_id_ = 0;
  }
}

CGAL_INLINE_FUNCTION
GLint Shader_program::uniform_location(const GLchar *name) const {
  auto it = cached_uniforms_.find(name);
  if (it != cached_uniforms_.end()) return it->second;
  GLint loc = glGetUniformLocation(program_id_, name);
  cached_uniforms_.emplace(name, loc);
  return loc;
}

CGAL_INLINE_FUNCTION
void Shader_program::uniform(const GLchar *name, const mat4f& matrix, GLboolean transpose) const {
  glUniformMatrix4fv(uniform_location(name), 1, transpose, matrix.data());
}

CGAL_INLINE_FUNCTION
void Shader_program::uniform(const GLchar *name, const vec4f& vector) const {
  glUniform4fv(uniform_location(name), 1, vector.data());
}

CGAL_INLINE_FUNCTION
void Shader_program::uniform(const GLchar *name, const vec3f& vector) const {
  glUniform3fv(uniform_location(name), 1, vector.data());
}

CGAL_INLINE_FUNCTION
void Shader_program::uniform(const GLchar *name, const vec2f& vector) const {
  glUniform2fv(uniform_location(name), 1, vector.data());
}

CGAL_INLINE_FUNCTION
void Shader_program::uniform(const GLchar *name, float value) const {
  glUniform1f(uniform_location(name), value);
}

CGAL_INLINE_FUNCTION
void Shader_program::uniform(const GLchar *name, int value) const {
  glUniform1i(uniform_location(name), value);
}

CGAL_INLINE_FUNCTION
GLuint Shader_program::compile_shader(GLenum stage, const GLchar *source) {
  GLuint shader = glCreateShader(stage);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  if (!Shader_program::check_shader_compile(shader, stage)) {
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

CGAL_INLINE_FUNCTION
Shader_program Shader_program::create(const char *source_vertex,
                      const char *source_fragment,
                      const char *source_geometry) {
  GLuint program = glCreateProgram();
  
  // VERTEX shader
  GLuint vs = Shader_program::compile_shader(GL_VERTEX_SHADER, source_vertex);
  if (vs == 0) { 
    glDeleteProgram(program); 
    return Shader_program{0}; 
  }
  glAttachShader(program, vs);
  glDeleteShader(vs);

  // GEOMETRY shader if exists
  if (source_geometry) {
    GLuint gs = Shader_program::compile_shader(GL_GEOMETRY_SHADER, source_geometry);
    if (gs == 0) { 
      glDeleteProgram(program); 
      return Shader_program{0}; 
    }
    glAttachShader(program, gs);
    glDeleteShader(gs);
  }

  // FRAGMENT shader
  GLuint fs = Shader_program::compile_shader(GL_FRAGMENT_SHADER, source_fragment);
  if (fs == 0) { 
    glDeleteProgram(program); 
    return Shader_program{0}; 
  }
  glAttachShader(program, fs);
  glDeleteShader(fs);

  glLinkProgram(program);
  if (!Shader_program::check_program_link(program)) {
    glDeleteProgram(program); 
    return Shader_program{0}; 
  }


  return Shader_program(program);
}

CGAL_INLINE_FUNCTION
bool Shader_program::check_shader_compile(GLuint shader, GLenum stage) {
  GLint success;
  GLchar info_log[INFO_LOG_SIZE];

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(shader, INFO_LOG_SIZE, nullptr, info_log);
    std::cerr
        << "ERROR::SHADER_COMPILATION_ERROR of type: " << stage_name(stage) << "\n"
        << info_log
        << "\n -- --------------------------------------------------- -- "
        << std::endl;
    return false;
  }


  return true; // successful compilation
}

CGAL_INLINE_FUNCTION
bool Shader_program::check_program_link(GLuint shader) {
  GLint success;
  GLchar info_log[INFO_LOG_SIZE];

  glGetProgramiv(shader, GL_LINK_STATUS, &success);

  if (!success) {
    glGetProgramInfoLog(shader, INFO_LOG_SIZE, nullptr, info_log);
    std::cerr
        << "ERROR::PROGRAM_LINKING_ERROR\n"
        << info_log
        << "\n -- --------------------------------------------------- -- "
        << std::endl;
    return false;
  }

  return true; // successful linkage
}


} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_SHADER_PROGRAM_IMPL_H
