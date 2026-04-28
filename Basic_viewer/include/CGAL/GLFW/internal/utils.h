#ifndef CGAL_GLFW_INTERNAL_UTILS_H
#define CGAL_GLFW_INTERNAL_UTILS_H

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>


namespace CGAL {
namespace GLFW {

using vec2i = Eigen::Vector2i;
using vec2f = Eigen::Vector2f;
using vec3f = Eigen::Vector3f;
using vec4f = Eigen::Vector4f;

using mat3f = Eigen::Matrix3f;
using mat4f = Eigen::Matrix4f;

using quatf = Eigen::Quaternionf;

namespace internal {

namespace utils {

bool inside_bounding_box_2d(const vec2f& point, const vec2f& pmin, const vec2f& pmax);

vec2f normalized_coordinates(const vec2f& coords, float width, float height);

bool equal_float(float a, float b, float epsilon = 0.001f);

bool equal_vec3f(vec3f u, vec3f v, float epsilon = 0.001f);

float radians(float x);

vec2f radians(const vec2f& v);

float degrees(float x);

mat4f euler_angle_xy(const float& angle_x, const float& angle_y);

mat4f perspective(float fov, float aspect, float z_near, float z_far);

mat4f ortho(float left, float right, float bottom, float top, float z_near, float z_far);

vec3f center(const vec3f& a, const vec3f& b);

float distance2(const vec3f& a, const vec3f& b);

float distance(const vec3f& a, const vec3f& b);

vec3f sub_vec(const vec3f& u, const vec3f& v);

vec3f mult_vec_mat(const vec3f& u, const mat4f& m);

vec3f lerp(const vec3f& u, const vec3f& v, const float t);

} // namespace utils

namespace transform {

mat4f rotation_x(float theta);

mat4f rotation_y(float theta);

mat4f rotation_z(float theta);

mat4f rotation(float theta, vec3f axis);

mat4f rotation(const quatf& quaternion);

mat4f translation(const vec3f& v);

mat4f translation(const float x, const float y, const float z);

mat4f viewport(const float width, const float height);

} // namespace transform
} // namespace internal
} // namespace GLFW
} // namespace CGAL

#ifdef CGAL_HEADER_ONLY
#include "utils_impl.h"
#endif // CGAL_HEADER_ONLY

#endif // CGAL_GLFW_INTERNAL_UTILS_H
