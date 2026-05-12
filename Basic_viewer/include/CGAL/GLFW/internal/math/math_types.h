#ifndef CGAL_GLFW_INTERNAL_MATH_TYPES_H
#define CGAL_GLFW_INTERNAL_MATH_TYPES_H

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Geometry>

namespace CGAL {
namespace GLFW {

using vec2i = Eigen::Vector2i;
using vec2f = Eigen::Vector2f;
using vec3f = Eigen::Vector3f;
using vec4f = Eigen::Vector4f;

using mat3f = Eigen::Matrix3f;
using mat4f = Eigen::Matrix4f;

using quatf = Eigen::Quaternionf;

}
}

#endif // CGAL_GLFW_INTERNAL_MATH_TYPES_H
