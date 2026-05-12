#ifndef CGAL_GLFW_INTERNAL_DRAWABLE_H
#define CGAL_GLFW_INTERNAL_DRAWABLE_H

#include <CGAL/basic.h>
#include <vector>

#include <glad/gl.h>

#include "Vertex_array.h"
#include "Gl_buffer.h"

namespace CGAL {
namespace GLFW {
namespace internal {

struct Vertex_attribute {
  GLuint location; 
  GLint components; 
  GLenum type = GL_FLOAT; 
  GLboolean normalized = GL_FALSE; 
};

class Drawable {
public: 
  Drawable() = default; 

  Drawable(const Drawable&) = delete; 
  Drawable& operator=(const Drawable&) = delete; 
  Drawable(Drawable&&) = default; 
  Drawable& operator=(Drawable&&) = default; 

  ~Drawable() = default;
  
  void configure(GLenum primitive_type, const std::vector<Vertex_attribute>& attributes);
  
  template<typename T>
  void upload(std::size_t i, const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW);

  void draw() const;

  bool is_valid() const { return vao_.is_valid() && vertex_count_ > 0; }
  GLsizei vertex_count() const { return vertex_count_; }
  GLenum primitive_type() const { return primitive_; }

private:
  static constexpr GLsizei type_size(GLenum type) {
    switch (type) {
      case GL_FLOAT:          return sizeof(GLfloat); 
      case GL_INT:            return sizeof(GLint); 
      case GL_UNSIGNED_INT:   return sizeof(GLuint); 
      case GL_UNSIGNED_BYTE:  return sizeof(GLubyte); 
      default:                return sizeof(GLfloat); 
    }
  } 

private: 

  std::vector<Gl_buffer> vbos_{};
  std::vector<Vertex_attribute> attributes_{};
  
  Vertex_array vao_{};
  
  GLenum primitive_{GL_TRIANGLES}; 
 
  GLsizei vertex_count_{0}; 
};

template<typename T>
void Drawable::upload(std::size_t i, const std::vector<T>& data, GLenum usage) {
  CGAL_precondition(i < vbos_.size());
  vbos_[i].upload(GL_ARRAY_BUFFER, data, usage);
  if (i == 0 && !attributes_.empty()) { // vbo of i==0 holds vertex positions
    const GLint comps = attributes_[0].components; 
    vertex_count_ = (comps > 0) ? 
      static_cast<GLsizei>(data.size() / static_cast<std::size_t>(comps)) : 0;    
  }  
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Drawable_impl.h"
#endif 

#endif // CGAL_GLFW_INTERNAL_DRAWABLE_H
