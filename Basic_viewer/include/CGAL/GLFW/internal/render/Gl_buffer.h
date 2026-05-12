#ifndef CGAL_GLFW_INTERNAL_GL_BUFFER_H
#define CGAL_GLFW_INTERNAL_GL_BUFFER_H

#include <glad/gl.h>

#include <utility>
#include <vector>

namespace CGAL {
namespace GLFW {
namespace internal {

class Gl_buffer {
public:
  Gl_buffer() { glGenBuffers(1, &id_); }

  Gl_buffer(const Gl_buffer&) = delete; 
  Gl_buffer& operator=(const Gl_buffer&) = delete; 
  Gl_buffer(Gl_buffer&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
  Gl_buffer& operator=(Gl_buffer&& other) noexcept {
    if (&other != this) {
      destroy();
      id_ = std::exchange(other.id_, 0);  
    }
    return *this; 
  }

  ~Gl_buffer() { destroy(); }

  void bind(GLenum target) const { glBindBuffer(target, id_); }
  static void unbind(GLenum target) { glBindBuffer(target, 0); }

  template<typename T>
  void upload(GLenum target, const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW) const {
    bind(target);
    glBufferData(target, data.size() * sizeof(T), data.data(), usage); 
  }

  GLuint id() const { return id_; }
  bool is_valid() const { return id_ != 0; }

private: 
  void destroy() { if (id_) glDeleteBuffers(1, &id_); } 

private: 
  GLuint id_{0};
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_GL_BUFFER_H
