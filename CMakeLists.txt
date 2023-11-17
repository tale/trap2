cmake_minimum_required(VERSION 3.19) # 3.19 introduced Apple Silicon support
include(FetchContent) # Enable FetchContent to clone dependencies

project(
	trap2
	VERSION 1.0.0 
	DESCRIPTION "A state-machine powered, GPU accelerated terminal"
	HOMEPAGE_URL "https://github.com/tale/trap2"
	LANGUAGES C
)

# Git auto-ignore out-of-source build directory
if(NOT PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	file(GENERATE OUTPUT .gitignore CONTENT "*")
endif()

include(cmake/glfw.cmake)
include(cmake/glew.cmake)
include(cmake/vterm.cmake)
include(cmake/freetype.cmake)

add_executable(trap2
	src/main.c
	src/log.c
	src/shader.c
	src/font.c
	src/term/init.c
	src/term/destroy.c
	src/term/handle.c
	src/term/render.c
	src/term/resize.c
)

# Silence GL deprecations if on macOS
if(APPLE)
	target_compile_definitions(trap2 PRIVATE
		GL_SILENCE_DEPRECATION
	)
endif()

target_include_directories(trap2 PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${freetype_SOURCE_DIR}/include
	${vterm_SOURCE_DIR}/include
	${glew_SOURCE_DIR}/include
	${glfw_SOURCE_DIR}/include
)

target_link_libraries(trap2 PRIVATE
	glfw
	libglew_static
	freetype
	vterm
)
