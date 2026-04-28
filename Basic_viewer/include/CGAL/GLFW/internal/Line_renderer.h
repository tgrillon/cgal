#ifndef CGAL_GLFW_INTERNAL_LINE_RENDERER_H
#define CGAL_GLFW_INTERNAL_LINE_RENDERER_H

#include <CGAL/config.h>

#include <vector>

#include <glad/gl.h>

#include "utils.h"

namespace CGAL {
namespace GLFW {
namespace internal {

class Line_renderer {
public:
  void delete_buffers();
  void initialize_buffers();
  void load_buffers();
  void add_line(const vec3f &start, const vec3f &end);
  void add_line(const vec3f &start, const vec3f &end, const vec3f &color);
  void draw();

  void generate_grid(float size, int nb_subdivisions = 10);
  void generate_grid(const vec3f &color, float size, int nb_subdivisions = 10);

  inline bool are_buffers_loaded() const { return are_buffers_loaded_; }
  inline bool are_buffers_initialized() const {
    return are_buffers_initialized_;
  }

  inline void set_width(const float width) { width_ = width; }

private:
  std::vector<float> vertices_{};

  float width_{1.0f};

  unsigned int vertex_array_{0};
  unsigned int vertex_buffer_{0};

  bool are_buffers_loaded_{false};
  bool are_buffers_initialized_{false};
};

} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "Line_renderer_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_LINE_RENDERER_H
