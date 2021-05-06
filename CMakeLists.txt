cmake_minimum_required(VERSION 3.18.2)
project(ShaderSanbox)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunused-function -lGL -lGLEW -lglfw")

file(GLOB_RECURSE SOURCE_FILES "./src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCE_FILES "./src/imgui/*.cpp")

add_executable(
	shader_sandbox
	${SOURCE_FILES}
	${IMGUI_SOURCE_FILES}
)
