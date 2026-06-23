#.rst:
# CGAL_SetupCGAL_GLFWDependencies
# -------------------------------
#
# The module searches for the dependencies of the `CGAL_GLFW` library used by the GLFW Basic viewer:
#   - the `glfw3` library (via its CMake config package)
#   - an `OpenGL` runtime
#   - the `Eigen3` library (>= 3.1.0)
#
# by calling
#
# .. code-block:: cmake
#
#    find_package(glfw3 CONFIG QUIET)
#    find_package(OpenGL QUIET)
#    find_package(Eigen3 3.1.0 QUIET)
#
# and defines the variable :variable:`CGAL_GLFW_FOUND` and the function
# :command:`CGAL_setup_CGAL_GLFW_dependencies`.
#
if(CGAL_SetupCGAL_GLFWDependencies_included)
  return()
endif()
set(CGAL_SetupCGAL_GLFWDependencies_included TRUE)

#.rst:
# Used Modules
# ^^^^^^^^^^^^
#   - :module:`glfw3Config` — provides ``glfw``                                                             
#   - :module:`FindOpenGL` — provides ``OpenGL::GL``                                                        
#   - :module:`FindEigen3` — provides ``Eigen3::Eigen``  
#
find_package(glfw3 CONFIG QUIET)
find_package(OpenGL QUIET)
find_package(Eigen3 3.1.0 QUIET)

set(CGAL_GLFW_MISSING_DEPS "")
if(NOT glfw3_FOUND)
  message(STATUS "NOTICE: NOT glfw3_FOUND")
  set(CGAL_GLFW_MISSING_DEPS "glfw3")
endif()
if(NOT OPENGL_FOUND)
  message(STATUS "NOTICE: NOT OPENGL_FOUND")
  set(CGAL_GLFW_MISSING_DEPS "${CGAL_GLFW_MISSING_DEPS} OpenGL")
endif()
if(NOT (EIGEN3_FOUND OR Eigen3_FOUND))
  message(STATUS "NOTICE: NOT EIGEN3_FOUND or Eigen3_FOUND")
  set(CGAL_GLFW_MISSING_DEPS "${CGAL_GLFW_MISSING_DEPS} Eigen3")
endif()

#.rst:
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# .. variable:: CGAL_GLFW_FOUND
#
#    Set to `TRUE` if the dependencies of `CGAL_GLFW` were found.
#
if(NOT CGAL_GLFW_MISSING_DEPS)
  set(CGAL_GLFW_FOUND TRUE)
  set_property(GLOBAL PROPERTY CGAL_GLFW_FOUND TRUE)
endif()
 
#.rst:                                                                                                      
# Imported Targets                                                              
# ^^^^^^^^^^^^^^^^
#
# Defined when ``CGAL_GLFW_FOUND`` is ``TRUE``:                                                             
#                                                                                                           
# ``CGAL::glad``                                                                                            
#   :prop_tgt:`STATIC` GL loader compiled from the vendored ``gl.c``.                                       
#   Built with :prop_tgt:`POSITION_INDEPENDENT_CODE` (shared-library                                        
#   consumers) and :prop_tgt:`EXCLUDE_FROM_ALL` (built only when                                            
#   actually linked).                                                                                       
#                                                                                                           
# ``CGAL::stb``                                                                 
#   ``INTERFACE`` target for the header-only ``stb_image_write.h``. 
#
if(CGAL_GLFW_FOUND AND NOT TARGET CGAL_glad)                                                                                                
  add_library(CGAL_glad STATIC
    ${CGAL_BASIC_VIEWER_PACKAGE_DIR}/include/CGAL/GLFW/vendor/glad/src/gl.c)                                                                         
  target_include_directories(CGAL_glad SYSTEM PUBLIC 
    ${CGAL_BASIC_VIEWER_PACKAGE_DIR}/include/CGAL/GLFW/vendor/glad/include)                                                                          
  set_target_properties(CGAL_glad PROPERTIES 
    POSITION_INDEPENDENT_CODE ON 
    EXCLUDE_FROM_ALL TRUE)                                                                                                                             
  add_library(CGAL::glad ALIAS CGAL_glad)                                                                                                              
endif()

if(CGAL_GLFW_FOUND AND NOT TARGET CGAL_stb)                                                                                                
  add_library(CGAL_stb INTERFACE)     
  target_include_directories(CGAL_stb SYSTEM INTERFACE 
    ${CGAL_BASIC_VIEWER_PACKAGE_DIR}/include/CGAL/GLFW/vendor/stb/include)                                                                     
  add_library(CGAL::stb ALIAS CGAL_stb)                                                                                                              
endif()

#.rst:
#
# Provided Functions
# ^^^^^^^^^^^^^^^^^^
#
# .. command:: CGAL_setup_CGAL_GLFW_dependencies
#
#   Link the target with the dependencies of `CGAL_GLFW`::
#
#     CGAL_setup_CGAL_GLFW_dependencies( target )
#
#   The dependencies are added using :command:`target_link_libraries` with the
#   ``INTERFACE`` keyword.
#
function(CGAL_setup_CGAL_GLFW_dependencies target)
  target_link_libraries(${target} INTERFACE CGAL::CGAL)
  target_link_libraries(${target} INTERFACE glfw)       # external 
  target_link_libraries(${target} INTERFACE OpenGL::GL) # external
  target_link_libraries(${target} INTERFACE CGAL::glad) # vendored, compiled 
  target_link_libraries(${target} INTERFACE CGAL::stb)  # vendored, header-only 

  if(TARGET Eigen3::Eigen)
    target_link_libraries(${target} INTERFACE Eigen3::Eigen)
  else()
    target_include_directories(${target} SYSTEM INTERFACE ${EIGEN3_INCLUDE_DIR})
  endif()

  # GLFW_INCLUDE_NONE prevents <GLFW/glfw3.h> from pulling <GL/gl.h>,                                     
  # so <glad/gl.h> stays the single source of GL symbols. Load-bearing. 
  target_compile_definitions(${target} INTERFACE GLFW_INCLUDE_NONE)   
endfunction()
