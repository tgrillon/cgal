#ifndef CGAL_GLFW_INTERNAL_LINE_RENDERER_H
#define CGAL_GLFW_INTERNAL_LINE_RENDERER_H

#include <CGAL/config.h>

#include <vector>

#include <glad/gl.h>

#include <CGAL/GLFW/internal/math/math_types.h>
#include <CGAL/GLFW/internal/render/Vertex_array.h>
#include <CGAL/GLFW/internal/render/Gl_buffer.h>

namespace CGAL {
namespace GLFW {
namespace internal {

struct Grid_data {
  vec3f normal = vec3f::UnitZ();
  float size = 1.0f; 
  unsigned int subdivisions = 10; 
  vec3f color = vec3f::Zero();
};

struct Line_data {
  vec3f start;
  vec3f end;
  vec3f color = vec3f::Zero();
};

class Line_buffer {
public: 
  Line_buffer& add_line(const Line_data& data); 
  Line_buffer& add_grid(const Grid_data& data = {}); 
  Line_buffer& width(float w);
  
  float width() const { return width_; } 
  size_t vertex_count() const { return data_.size() / 6; }
  size_t data_count() const { return data_.size(); } 
  const float* vertex_data() const { return data_.data(); }; 
  const std::vector<float>& vertex_vector() const { return data_; }; 

private:
  std::vector<float> data_{}; // 6 components per vertex (3 for its position + 3 for its color)   
  float width_{1.0f}; 
};

class Line_renderer {
public:
  Line_renderer() = default; 

  Line_renderer(const Line_renderer&) = delete; 
  Line_renderer& operator=(const Line_renderer&) = delete; 

  Line_renderer(Line_renderer&& other) noexcept = default; 
  Line_renderer& operator=(Line_renderer&& other) noexcept = default; 

  ~Line_renderer() = default; 
  
  void draw() const;

  bool is_valid() const { return vao_.is_valid() && vertex_count_ > 0; } 
  
public:
  static Line_renderer create(const Line_buffer& data); 

private:
  Vertex_array vao_{};
  Gl_buffer vbo_{};

  float width_{1.0f};
  size_t vertex_count_{0};
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Line_renderer_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_LINE_RENDERER_H
