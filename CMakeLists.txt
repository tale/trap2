cmake_minimum_required(VERSION 3.19) # 3.19 introduced Apple Silicon support
include(FetchContent) # Enable FetchContent to clone dependencies

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
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

include(cmake/dawn.cmake)
include(cmake/glew.cmake)
include(cmake/vterm.cmake)
include(cmake/freetype.cmake)

add_executable(trap2
	src/main.c
	src/config.c
	src/log.c
	src/shader.c
	src/term/init.c
	src/term/destroy.c
	src/term/handle.c
	src/term/render.c
	src/term/resize.c
	src/term/render/cell.c
	src/term/render/font.c
	src/term/render/main.c
	src/term/render/util.c
	src/platform/mac.m
)

set_property(TARGET trap2 PROPERTY C_STANDARD 99)
set_property(TARGET trap2 PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET trap2 PROPERTY C_EXTENSIONS OFF)

# Silence GL deprecations if on macOS
if(APPLE)
	target_compile_definitions(trap2 PRIVATE
		GL_SILENCE_DEPRECATION
	)

	if (POLICY CMP0025)
		cmake_policy(SET CMP0025 NEW)
	endif()
endif()

target_include_directories(trap2 PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${freetype_SOURCE_DIR}/include
	${vterm_SOURCE_DIR}/include
	${glew_SOURCE_DIR}/include
	${glfw_SOURCE_DIR}/include
	${dawn_SOURCE_DIR}/include
)

target_link_libraries(trap2 PRIVATE
	webgpu
	glfw
	libglew_static
	freetype
	vterm
)
