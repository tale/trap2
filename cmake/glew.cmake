message(STATUS "Fetching glew")
FetchContent_Declare(
	glew
	GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
	GIT_TAG glew-cmake-2.2.0
)

FetchContent_GetProperties(glew)
if(NOT glew_POPULATED)
	FetchContent_Populate(glew)

	set(glew-cmake_BUILD_SHARED OFF CACHE INTERNAL "Build the shared glew library")
	set(ONLY_LIBS ON CACHE INTERNAL "Only build the glew library")
	add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR})
endif()

