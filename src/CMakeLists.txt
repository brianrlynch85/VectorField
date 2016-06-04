# ------------------------------------------------------------------------
#
#                             CMakeLists.txt for vector_field
#                                        V 0.01
#
#                            (c) Brian Lynch October, 2015
#
# ------------------------------------------------------------------------
cmake_minimum_required (VERSION 2.8)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

#Make sure OpenGL and GLUT are installed
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

#Set the executable vector_field.cpp source dependencies
set(vfmon_src vector_field.cpp)

#Add the executable, which will be in build/bin
add_executable(vector_field ${vfmon_src})

#Link the openGL libs
target_link_libraries(vector_field ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})