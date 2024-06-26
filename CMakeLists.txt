# Minimum cmake version required
cmake_minimum_required(VERSION 3.8)

# Project configuration
project(Testbed)

# OpenCV configuration
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Disable building extras of nanogui we won't need (pure C++ project)
set(NANOGUI_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
set(NANOGUI_BACKEND       OpenGL CACHE BOOL " " FORCE)

# Disable building extras of pybind11 we won't need (pure C++ project)
set(PYBIND11_INSTALL OFF CACHE BOOL " " FORCE)

# ---- Make sure to recursively clone all the git submodules for external libraries (nanogui) --- #
find_package(Git QUIET)
if(GIT_FOUND)
    if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    	# Update submodules as needed
    	option(CLONE_GIT_SUBMODULES "Check submodules during build" ON)
    	if(CLONE_GIT_SUBMODULES)
        	message(STATUS "Git Submodules update")
		message(STATUS "Working directory ${CMAKE_CURRENT_SOURCE_DIR}")
        	execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        	RESULT_VARIABLE GIT_SUBMOD_RESULT)
        	if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            	    message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        	endif()
    	endif()
    else()
  	message(FATAL_ERROR "This is not a Git repository. In order to build the testbed application, you need to clone the ReactPhysics3D repository.")
    endif()
else()
  message(FATAL_ERROR "Git has not been found on your system. This is necessary to build the testbed application because git submodules command is used to get the testbed dependencies (nanogui library). You need to fill in the GIT_EXECUTABLE CMake variable with the path to the Git executable on your system to continue.")
endif()

# Ask Nanogui not to build shared libraries
SET(NANOGUI_BUILD_SHARED OFF CACHE BOOL "Build Nanogui with static libraries" FORCE)

# Add the configurations from nanogui
add_subdirectory(extern/nanogui)

# Add the configurations from pybind11
add_subdirectory(extern/pybind11)

# Copy the shaders used for the demo into the build directory
file(COPY "shaders/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

# Copy the meshes used for the demo into the build directory
file(COPY "meshes/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/meshes/")

# OpenGLFramework source files
set(OPENGLFRAMEWORK_SOURCES
    opengl-framework/src/maths/Color.h
    opengl-framework/src/maths/Matrix3.h
    opengl-framework/src/maths/Matrix4.h
    opengl-framework/src/maths/Vector2.h
    opengl-framework/src/maths/Vector3.h
    opengl-framework/src/maths/Vector4.h
    opengl-framework/src/Camera.cpp
    opengl-framework/src/Camera.h
    opengl-framework/src/definitions.h
    opengl-framework/src/FrameBufferObject.cpp
    opengl-framework/src/FrameBufferObject.h
    opengl-framework/src/Light.h
    opengl-framework/src/Light.cpp
    opengl-framework/src/Mesh.h
    opengl-framework/src/Mesh.cpp
    opengl-framework/src/MeshReaderWriter.h
    opengl-framework/src/MeshReaderWriter.cpp
    opengl-framework/src/Object3D.h
    opengl-framework/src/Object3D.cpp
    opengl-framework/src/openglframework.h
    opengl-framework/src/Shader.h
    opengl-framework/src/Shader.cpp
    opengl-framework/src/Texture2D.h
    opengl-framework/src/Texture2D.cpp
    opengl-framework/src/TextureReaderWriter.h
    opengl-framework/src/TextureReaderWriter.cpp
    opengl-framework/src/VertexBufferObject.h
    opengl-framework/src/VertexBufferObject.cpp
    opengl-framework/src/VertexArrayObject.h
    opengl-framework/src/VertexArrayObject.cpp
)

# Testbed source files
set(TESTBED_SOURCES
    src/Main.cpp
    src/TestbedApplication.h
    src/TestbedApplication.cpp
    src/Gui.h
    src/Gui.cpp
    src/Scene.h
    src/Scene.cpp
    src/SceneDemo.h
    src/SceneDemo.cpp
    src/Timer.h
    src/TestbedLogger.h
    src/TestbedLogger.cpp
		src/VideoController.cpp
		src/VideoController.h
)

# Common source files
set(COMMON_SOURCES
    common/Box.h
    common/Box.cpp
    common/Sphere.h
    common/Sphere.cpp
    common/Line.h
    common/Line.cpp
    common/Capsule.h
    common/Capsule.cpp
    common/ConvexMesh.h
    common/ConvexMesh.cpp
    common/ConcaveMesh.h
    common/ConcaveMesh.cpp
    common/Dumbbell.h
    common/Dumbbell.cpp
    common/HeightField.h
    common/HeightField.cpp
    common/PhysicsObject.h
    common/PhysicsObject.cpp
    common/VisualContactPoint.h
    common/VisualContactPoint.cpp
    common/PerlinNoise.h
    common/PerlinNoise.cpp
    common/AABB.h
    common/AABB.cpp
        common/Skeleton.cpp
		common/BVH.cpp
        common/Bone.cpp
        common/Bone.h
		common/Analysizer.cpp
		common/Analysizer.h
		common/Identifier.cpp
		common/Identifier.h
		common/VideoToBvhConverter.cpp
		common/VideoToBvhConverter.h
)

# Scenes source files
set(SCENES_SOURCES
    scenes/bvh_viewer/BvhScene.h
    scenes/bvh_viewer/BvhScene.cpp
)

# Utils source files
set(UTILS_SOURCES
		utils/AngleTool.cpp
		utils/AngleTool.h
		utils/Event.cpp
		utils/Event.h
		utils/debug.h
)

# Create the executable
add_executable(testbed ${TESTBED_SOURCES} ${SCENES_SOURCES} ${COMMON_SOURCES} ${OPENGLFRAMEWORK_SOURCES} ${UTILS_SOURCES})

# Headers
target_include_directories(testbed PRIVATE
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/opengl-framework/src>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/common>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/scenes>
              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extern/nanogui/include>
		      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
)
foreach(p ${NANOGUI_EXTRA_INCS})
    target_include_directories(testbed PRIVATE $<1:${p}>)
endforeach()


# Compile definitions
target_compile_definitions(testbed PRIVATE ${NANOGUI_EXTRA_DEFS})

# Handel DEBUG and RELEASE mode
if (CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_definitions(testbed PRIVATE -DDEBUG)
	message(STATUS "Debug mode")
endif()

# C++17 compiler features
target_compile_features(testbed PUBLIC cxx_std_17)
set_target_properties(testbed PROPERTIES CXX_EXTENSIONS OFF)

# Link with libraries
target_link_libraries(testbed reactphysics3d nanogui ${NANOGUI_EXTRA_LIBS} ${OpenCV_LIBS} pybind11::embed)

# Copy the python packages into the build directory
add_custom_target(copy_python_packages ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Py_package ${CMAKE_CURRENT_BINARY_DIR}/Py_package
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Py_package)
add_dependencies(testbed copy_python_packages)

# Copy static files into the build directory
add_custom_target(copy_static_files ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/static ${CMAKE_CURRENT_BINARY_DIR}/static
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/static)
add_dependencies(testbed copy_static_files)