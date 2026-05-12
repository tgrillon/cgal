#ifndef CGAL_GLFW_INTERNAL_DRAWABLE_IMPL_H
#define CGAL_GLFW_INTERNAL_DRAWABLE_IMPL_H

#include "Vertex_array.h"
#include <CGAL/config.h>
#include <cstddef>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Drawable.h"

#else 
#define CGAL_INLINE_FUNCTION
#endif 

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
void Drawable::configure(GLenum primitive_type, const std::vector<Vertex_attribute>& attributes) {
  primitive_ = primitive_type; 
  attributes_ = attributes; 

  vbos_.clear(); 
  vbos_.reserve(attributes_.size()); 
  for (std::size_t ii = 0; ii < attributes_.size(); ++ii) {
    vbos_.emplace_back(); 
    const auto& attr = attributes_[ii]; 
    vbos_[ii].bind(GL_ARRAY_BUFFER); 
    vao_.set_attribute(attr.location, attr.components, 
                       attr.type, attr.normalized, 
                       attr.components * type_size(attr.type)); 
  }
  Vertex_array::unbind();
}

CGAL_INLINE_FUNCTION
void Drawable::draw() const {
  if (vertex_count_ == 0) return; 
  vao_.bind();
  glDrawArrays(primitive_, 0, vertex_count_); 
  Vertex_array::unbind();
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_DRAWABLE_IMPL_H
