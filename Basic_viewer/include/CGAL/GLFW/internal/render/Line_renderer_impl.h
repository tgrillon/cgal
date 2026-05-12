#ifndef CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H
#define CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H

#include <CGAL/basic.h>
#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Line_renderer.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

namespace CGAL {
namespace GLFW {
namespace internal {

//-------------- LINE BUFFER IMPL --------------

CGAL_INLINE_FUNCTION
Line_buffer& Line_buffer::add_line(const Line_data& data) {
  auto push = [&](const vec3f &position) {
    data_.emplace_back(position.x());
    data_.emplace_back(position.y());
    data_.emplace_back(position.z());
    data_.emplace_back(data.color.x());
    data_.emplace_back(data.color.y());
    data_.emplace_back(data.color.z());
  };

  push(data.start); 
  push(data.end); 

  return *this;
}

CGAL_INLINE_FUNCTION
Line_buffer& Line_buffer::add_grid(const Grid_data& data) {
  CGAL_precondition(data.subdivisions > 0); 
  if (data.subdivisions == 0) return *this;

  const auto& n = data.normal; 
  const vec3f u = n.unitOrthogonal();  
  const vec3f v = n.cross(u).normalized();  

  const float div = 1.0 / data.subdivisions;  
  for (unsigned int i = 0; i <= data.subdivisions; ++i) {
    float pos = float(data.size * (2.0 * i * div - 1.0));
    add_line({ .start = pos * u - data.size * v, .end = pos * u + data.size * v, .color = data.color });
    add_line({ .start = pos * v - data.size * u, .end = pos * v + data.size * u, .color = data.color });
  }
  return *this;
}

CGAL_INLINE_FUNCTION
Line_buffer& Line_buffer::width(float w) {
  width_ = w; 
  return *this;
}

//-------------- LINE RENDERER IMPL --------------

CGAL_INLINE_FUNCTION
Line_renderer Line_renderer::create(const Line_buffer& data) {
  Vertex_array vao; 
  Gl_buffer vbo; 

  vao.bind(); 
  vbo.bind(GL_ARRAY_BUFFER);
  vbo.upload<float>(GL_ARRAY_BUFFER, data.vertex_vector());

  // vertex attribute
  vao.set_attribute(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float)); 
  // color attribute
  vao.set_attribute(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 12); 

  vao.unbind(); 
  vbo.unbind(GL_ARRAY_BUFFER);

  Line_renderer renderer;
  renderer.vao_ = std::move(vao);  
  renderer.vbo_ = std::move(vbo);  
  renderer.width_ = data.width();  
  renderer.vertex_count_ = data.vertex_count();  
  return renderer; 
}

CGAL_INLINE_FUNCTION
void Line_renderer::draw() const {
  CGAL_precondition_msg(is_valid(), 
    "Line_renderer must be initialized using Line_renderer::create before calling Line_renderer::draw");
  
  vao_.bind();

  glLineWidth(width_);
  glDrawArrays(GL_LINES, 
               0, 
               static_cast<GLsizei>(vertex_count_));
  
  vao_.unbind();
  glLineWidth(1.0f);
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H
