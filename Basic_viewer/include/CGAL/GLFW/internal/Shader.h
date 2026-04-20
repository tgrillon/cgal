#ifndef CGAL_GLFW_INTERNAL_SHADER_H
#define CGAL_GLFW_INTERNAL_SHADER_H

#include <iostream>
#include <memory>
#include <unordered_map>

#include <glad/gl.h>

namespace CGAL {
namespace GLFW {
namespace internal {

class Shader {
public:
  Shader() = default;
  explicit Shader(const GLuint program_id);

  void use() const;
  void destroy();
  bool is_valid() const;

  GLint get_uniform_location(const GLchar *name);

  void set_mat4f(const GLchar *name, const GLfloat *data,
                 GLboolean transpose = false);
  void set_vec4f(const GLchar *name, const GLfloat *data);
  void set_vec3f(const GLchar *name, const GLfloat *data);
  void set_vec2f(const GLchar *name, const GLfloat *data);
  void set_float(const GLchar *name, const float data);
  void set_int(const GLchar *name, const int data);

  static Shader create(const GLchar *source_vertex,
                       const GLchar *source_fragment,
                       const GLchar *source_geometry = nullptr);

  static GLchar *enum_type_to_str(const GLenum type);

private:
  static int check_compile_errors(const GLuint shader_id, const GLchar *type);
  static GLuint compile_shader(const GLenum type, const GLchar *source);

private:
  std::unordered_map<const GLchar *, int> cached_uniforms_{};
  GLuint program_id_{0};
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Shader_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_SHADER_H
