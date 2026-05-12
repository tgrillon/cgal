#ifndef CGAL_GLFW_INTERNAL_VERTEX_ARRAY_H
#define CGAL_GLFW_INTERNAL_VERTEX_ARRAY_H

#include <glad/gl.h>

#include <utility>

namespace CGAL {
namespace GLFW {
namespace internal {

class Vertex_array {
public:  
  Vertex_array()  { glGenVertexArrays(1, &id_); }
  
  Vertex_array(const Vertex_array&) = delete;
  Vertex_array& operator=(const Vertex_array&) = delete;
  
  Vertex_array(Vertex_array&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
  Vertex_array& operator=(Vertex_array&& other) noexcept {                                                        
    if (&other != this) { 
      destroy(); 
      id_ = std::exchange(other.id_, 0); 
    }
    return *this; 
  }

  ~Vertex_array() { destroy(); }
  
  void bind() const   { glBindVertexArray(id_); }                                           
  static void unbind(){ glBindVertexArray(0); }
  
  // Records into the currently-bound VAO; caller must have bound a VBO to GL_ARRAY_BUFFER.
  void set_attribute(GLuint location, GLint components, 
                     GLenum type = GL_FLOAT, GLboolean normalized = GL_FALSE, 
                     GLsizei stride = 0, std::size_t offset = 0) const {
    bind();
    glVertexAttribPointer(location, components, type, normalized, stride,
                          reinterpret_cast<const void*>(offset));
    glEnableVertexAttribArray(location);
  }
  
  GLuint id() const { return id_; }
  bool is_valid() const { return id_ != 0; }                                                

private: 
  void destroy() { if (id_) glDeleteVertexArrays(1, &id_); }

private:
  GLuint id_{0};
};

} // namespace internal 
} // namespace GLFW
} // namespace CGAL 


#endif // CGAL_GLFW_INTERNAL_VERTEX_ARRAY_H
