#ifndef CGAL_GLFW_INTERNAL_SHADER_PROGRAM_H
#define CGAL_GLFW_INTERNAL_SHADER_PROGRAM_H

#include <unordered_map>
#include <string_view>
#include <string>

#include <glad/gl.h>

#include <CGAL/GLFW/internal/math/math_types.h>

namespace CGAL {
namespace GLFW {
namespace internal {

class Shader_program {
public:
  explicit Shader_program(const GLuint program_id = 0) : program_id_(program_id) {}

  Shader_program(const Shader_program&) = delete; 
  Shader_program& operator=(const Shader_program&) = delete; 

  Shader_program(Shader_program&&) noexcept; 
  Shader_program& operator=(Shader_program&&) noexcept; 

  ~Shader_program() { destroy(); }

  void use() const { glUseProgram(program_id_); }
  bool is_valid() const { return program_id_ != 0; }

  GLint uniform_location(const GLchar *name) const;

  void uniform(const GLchar *name, const mat4f& matrix, GLboolean transpose = false) const;
  void uniform(const GLchar *name, const vec4f& vector) const;
  void uniform(const GLchar *name, const vec3f& vector) const;
  void uniform(const GLchar *name, const vec2f& vector) const;
  void uniform(const GLchar *name, float value) const;
  void uniform(const GLchar *name, int value) const;

  static Shader_program create(const char *source_vertex,
                       const char *source_fragment,
                       const char *source_geometry = nullptr);         

private: 
  void destroy();
  static constexpr std::string_view stage_name(GLenum stage) {
    switch (stage) {
    case GL_VERTEX_SHADER:
      return "VERTEX";
    case GL_GEOMETRY_SHADER:
      return "GEOMETRY";
    case GL_FRAGMENT_SHADER:
      return "FRAGMENT";
    default:  
      return "UNKNOWN";
    }
  }

private: // static private methods
  static bool check_shader_compile(GLuint shader, GLenum stage);
  static bool check_program_link(GLuint shader);
  static GLuint compile_shader(GLenum stage, const GLchar *source);

private:
  mutable std::unordered_map<std::string, GLint> cached_uniforms_{};
  GLuint program_id_{0};

private: 
  static constexpr size_t INFO_LOG_SIZE = 1024; 
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Shader_program_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_SHADER_PROGRAM_H
