#ifndef CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H
#define CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "Line_renderer.h"

#else
#define CGAL_INLINE_FUNCTION
#endif // CGAL_HEADER_ONLY

#include <cassert>

namespace CGAL {
namespace GLFW {
namespace internal {

CGAL_INLINE_FUNCTION
void Line_renderer::delete_buffers() {
  if (vertex_array_ != 0)
    glDeleteVertexArrays(1, &vertex_array_);
  if (vertex_buffer_ != 0)
    glDeleteBuffers(1, &vertex_buffer_);
}

CGAL_INLINE_FUNCTION
void Line_renderer::initialize_buffers() {
  glGenVertexArrays(1, &vertex_array_);
  glGenBuffers(1, &vertex_buffer_);

  glBindVertexArray(vertex_array_);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

  // vertex attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);

  // color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (const void *)12);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  are_buffers_initialized_ = true;
}

CGAL_INLINE_FUNCTION
void Line_renderer::load_buffers() {
  assert(are_buffers_initialized());

  glBindVertexArray(vertex_array_);

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float),
               vertices_.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  are_buffers_loaded_ = true;
}

CGAL_INLINE_FUNCTION
void Line_renderer::add_line(const vec3f &start, const vec3f &end) {
  vertices_.emplace_back(start.x());
  vertices_.emplace_back(start.y());
  vertices_.emplace_back(start.z());
  vertices_.emplace_back(0.);
  vertices_.emplace_back(0.);
  vertices_.emplace_back(0.);

  vertices_.emplace_back(end.x());
  vertices_.emplace_back(end.y());
  vertices_.emplace_back(end.z());
  vertices_.emplace_back(0.);
  vertices_.emplace_back(0.);
  vertices_.emplace_back(0.);
}

CGAL_INLINE_FUNCTION
void Line_renderer::add_line(const vec3f &start, const vec3f &end,
                             const vec3f &color) {
  vertices_.emplace_back(start.x());
  vertices_.emplace_back(start.y());
  vertices_.emplace_back(start.z());
  vertices_.emplace_back(color.x());
  vertices_.emplace_back(color.y());
  vertices_.emplace_back(color.z());

  vertices_.emplace_back(end.x());
  vertices_.emplace_back(end.y());
  vertices_.emplace_back(end.z());
  vertices_.emplace_back(color.x());
  vertices_.emplace_back(color.y());
  vertices_.emplace_back(color.z());
}

CGAL_INLINE_FUNCTION
void Line_renderer::draw() {
  assert(are_buffers_loaded());

  if (vertices_.empty())
    return;

  unsigned int n_components =
      6; // 6 components per vertex (3 for position + 3 for color)

  glBindVertexArray(vertex_array_);

  glLineWidth(width_);
  glDrawArrays(GL_LINES, 0,
               static_cast<GLsizei>(vertices_.size() / n_components));

  glBindVertexArray(0);
  glLineWidth(1.f);
}

CGAL_INLINE_FUNCTION
void Line_renderer::generate_grid(float size, int nb_subdivisions) {
  for (unsigned int i = 0; i <= nb_subdivisions; ++i) {
    float pos = float(size * (2.0 * i / nb_subdivisions - 1.0));
    add_line(vec3f(pos, -size, 0.f), vec3f(pos, size, 0.f));
    add_line(vec3f(-size, pos, 0.f), vec3f(size, pos, 0.f));
  }
}

CGAL_INLINE_FUNCTION
void Line_renderer::generate_grid(const vec3f &color, float size, int nb_subdivisions) {
  for (unsigned int i = 0; i <= nb_subdivisions; ++i) {
    float pos = float(size * (2.0 * i / nb_subdivisions - 1.0));
    add_line(vec3f(pos, -size, 0.f), vec3f(pos, size, 0.f), color);
    add_line(vec3f(-size, pos, 0.f), vec3f(size, pos, 0.f), color);
  }
}

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#endif // CGAL_GLFW_INTERNAL_LINE_RENDERER_IMPL_H
