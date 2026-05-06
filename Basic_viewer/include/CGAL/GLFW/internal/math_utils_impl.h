#ifndef CGAL_GLFW_INTERNAL_MATH_UTILS_IMPL_H
#define CGAL_GLFW_INTERNAL_MATH_UTILS_IMPL_H

#include <CGAL/config.h>

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline

#include "math_utils.h"

#else 
#define CGAL_INLINE_FUNCTION
#endif 

namespace CGAL {
namespace GLFW {
namespace internal {

namespace utils {

CGAL_INLINE_FUNCTION
bool inside_bounding_box_2d(const vec2f& point, const vec2f& pmin, const vec2f& pmax) {
  return point.x() >= pmin.x() && point.x() <= pmax.x() && point.y() >= pmin.y() && point.y() <= pmax.y();
}

CGAL_INLINE_FUNCTION
vec2f normalized_coordinates(const vec2f& coords, float width, float height) {
  vec2f result;
  result.x() = coords.x() / width * 2 - 1;
  result.y() = -1 * (coords.y() / height * 2 - 1);

  return result;
}

CGAL_INLINE_FUNCTION
bool equal_float(float a, float b, float epsilon) {
  return std::fabs(a - b) < epsilon;
}

CGAL_INLINE_FUNCTION
bool equal_vec3f(vec3f u, vec3f v, float epsilon) {
  return equal_float(u.x(), v.x(), epsilon) && equal_float(u.y(), v.y(), epsilon) && equal_float(u.z(), v.z(), epsilon);
}

CGAL_INLINE_FUNCTION
float radians(float x) {
  return M_PI / 180 * x;
}

CGAL_INLINE_FUNCTION
vec2f radians(const vec2f& v) {
  vec2f result;
  result << radians(v.x()),
      radians(v.y());
  return result;
}

CGAL_INLINE_FUNCTION
float degrees(float x) {
  return 180 / M_PI * x;
}

CGAL_INLINE_FUNCTION
mat4f euler_angle_xy(const float& angle_x, const float& angle_y) {
  float cos_x = std::cos(angle_x);
  float sin_x = std::sin(angle_x);
  float cos_y = std::cos(angle_y);
  float sin_y = std::sin(angle_y);

  mat4f result = mat4f::Identity();
  result(0, 0) = cos_y;
  result(1, 0) = -sin_x * -sin_y;
  result(2, 0) = cos_x * -sin_y;
  result(1, 1) = cos_x;
  result(2, 1) = sin_x;
  result(0, 2) = sin_y;
  result(1, 2) = -sin_x * cos_y;
  result(2, 2) = cos_x * cos_y;

  return result;
}

CGAL_INLINE_FUNCTION
mat4f perspective(float fov, float aspect, float z_near, float z_far) {
  assert(std::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0);

  const float tan_half_fov = std::tan(fov * 0.5);
  mat4f result = mat4f::Zero();
  result(0, 0) = 1.0 / (aspect * tan_half_fov);
  result(1, 1) = 1.0 / (tan_half_fov);
  result(2, 2) = -(z_far + z_near) / (z_far - z_near);
  result(2, 3) = -(2.0 * z_far * z_near) / (z_far - z_near);
  result(3, 2) = -1.0;

  return result;
}

CGAL_INLINE_FUNCTION
mat4f ortho(float left, float right, float bottom, float top, float z_near, float z_far) {
  mat4f result = mat4f::Identity();
  result(0, 0) = 2.0 / (right - left);
  result(1, 1) = 2.0 / (top - bottom);
  result(2, 2) = -2.0 / (z_far - z_near);
  result(0, 3) = -(right + left) / (right - left);
  result(1, 3) = -(top + bottom) / (top - bottom);
  result(2, 3) = -(z_far + z_near) / (z_far - z_near);

  return result;
}

CGAL_INLINE_FUNCTION
vec3f center(const vec3f& a, const vec3f& b) {
  vec3f ret;
  ret.x() = (a.x() + b.x()) * 0.5f;
  ret.y() = (a.y() + b.y()) * 0.5f;
  ret.z() = (a.z() + b.z()) * 0.5f;

  return ret;
}

CGAL_INLINE_FUNCTION
float distance2(const vec3f& a, const vec3f& b) {
  float dx = (b.x() - a.x());
  float dy = (b.y() - a.y());
  float dz = (b.z() - a.z());

  return dx * dx + dy * dy + dz * dz;
}

CGAL_INLINE_FUNCTION
float distance(const vec3f& a, const vec3f& b) {
  return std::sqrt(distance2(a, b));
}

CGAL_INLINE_FUNCTION
vec3f sub_vec(const vec3f& u, const vec3f& v) {
  vec3f w;
  w.x() = v.x() - u.x();
  w.y() = v.y() - u.y();
  w.z() = v.z() - u.z();

  return w;
}

CGAL_INLINE_FUNCTION
vec3f mult_vec_mat(const vec3f& u, const mat4f& m) {
  vec4f v = u.homogeneous();
  float x = m.row(0).dot(v);
  float y = m.row(1).dot(v);
  float z = m.row(2).dot(v);
  float w = m.row(3).dot(v);

  assert(w != 0);
  float d = 1.f / w;
  if (w == 1.f)
    return vec3f(x, y, z);
  return vec3f(x * d, y * d, z * d);

  return u;
}

CGAL_INLINE_FUNCTION
vec3f lerp(const vec3f& u, const vec3f& v, const float t) {
  return u * (1 - t) + v * t;
}

} // namespace utils

namespace transform {

CGAL_INLINE_FUNCTION
mat4f rotation_x(float theta) {
  Eigen::Affine3f transform{Eigen::AngleAxisf(theta, Eigen::Vector3f::UnitX()).toRotationMatrix()};
  return transform.matrix();
}

CGAL_INLINE_FUNCTION
mat4f rotation_y(float theta) {
  Eigen::Affine3f transform{Eigen::AngleAxisf(theta, Eigen::Vector3f::UnitY()).toRotationMatrix()};
  return transform.matrix();
}

CGAL_INLINE_FUNCTION
mat4f rotation_z(float theta) {
  Eigen::Affine3f transform{Eigen::AngleAxisf(theta, Eigen::Vector3f::UnitZ()).toRotationMatrix()};
  return transform.matrix();
}

CGAL_INLINE_FUNCTION
mat4f rotation(float theta, vec3f axis) {
  Eigen::Affine3f transform{Eigen::AngleAxisf(theta, axis).toRotationMatrix()};
  return transform.matrix();
}

CGAL_INLINE_FUNCTION
mat4f rotation(const quatf& quaternion) {
  mat3f rotation_3x3 = quaternion.toRotationMatrix();
  mat4f rotation = mat4f::Identity();
  rotation.block<3, 3>(0, 0) = rotation_3x3;

  return rotation;
}

CGAL_INLINE_FUNCTION
mat4f translation(const vec3f& v) {
  Eigen::Affine3f transform{Eigen::Translation3f(v)};
  return transform.matrix();
}

CGAL_INLINE_FUNCTION
mat4f translation(const float x, const float y, const float z) {
  vec3f v(x, y, z);
  Eigen::Affine3f transform{Eigen::Translation3f(v)};
  return transform.matrix();
}

CGAL_INLINE_FUNCTION
mat4f viewport(const float width, const float height) {
  float w = width * .5f;
  float h = height * .5f;

  mat4f ret;
  ret << w, 0, 0, w,
      0, h, 0, h,
      0, 0, .5f, .5f,
      0, 0, 0, 1.f;

  return ret;
}

} // namespace transform

} // namespace internal
} // namespace GLFW
} // namespace CGAL


#endif // CGAL_GLFW_INTERNAL_MATH_UTILS_IMPL_H
