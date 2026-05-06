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

#include "math_types.h"

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
Line_renderer::Line_renderer(Line_renderer&& other) noexcept 
  : vao_(other.vao_), vbo_(other.vbo_), width_(other.width_), vertex_count_(other.vertex_count_) {
  other.vao_ = 0; 
  other.vbo_ = 0;  
}

CGAL_INLINE_FUNCTION
Line_renderer& Line_renderer::operator=(Line_renderer&& other) noexcept {
  if (&other != this) {
    delete_buffers(); 
    vao_ = other.vao_; 
    other.vao_ = 0; 
    vbo_ = other.vbo_;
    other.vbo_ = 0;
      
    width_ = other.width_;
    vertex_count_ = other.vertex_count_;
  }
  return *this; 
}

CGAL_INLINE_FUNCTION
Line_renderer::~Line_renderer() {
  delete_buffers();
}

CGAL_INLINE_FUNCTION
Line_renderer Line_renderer::create(const Line_buffer& buffer) {
  GLuint vao, vbo; 
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 
               buffer.vertex_count() * 6 * sizeof(float), 
               buffer.vertex_data(), GL_STATIC_DRAW);

  // vertex attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
                        6 * sizeof(float), nullptr);

  // color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
                        6 * sizeof(float), reinterpret_cast<const void *>(12));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  Line_renderer renderer;
  renderer.vao_ = vao;  
  renderer.vbo_ = vbo;  
  renderer.width_ = buffer.width();  
  renderer.vertex_count_ = buffer.vertex_count();  
  return renderer; 
}

CGAL_INLINE_FUNCTION
void Line_renderer::draw() const {
  glBindVertexArray(vao_);

  glLineWidth(width_);
  glDrawArrays(GL_LINES, 
               0, 
               static_cast<GLsizei>(vertex_count_));
  
  glBindVertexArray(0);
  glLineWidth(1.0f);
}

CGAL_INLINE_FUNCTION
bool Line_renderer::is_valid() const {
  return vao_ != 0; 
} 

CGAL_INLINE_FUNCTION
void Line_renderer::delete_buffers() {
  glDeleteVertexArrays(1, &vao_);
  vao_ = 0; 
  glDeleteBuffers(1, &vbo_);
  vbo_ = 0; 
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H
