message(STATUS "Fetching dawn")
FetchContent_Declare(
	webgpu
	GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
	GIT_TAG dawn
)

FetchContent_GetProperties(webgpu)
if(NOT webgpu_POPULATED)
	FetchContent_Populate(webgpu)
	add_subdirectory(${webgpu_SOURCE_DIR} ${webgpu_BINARY_DIR})
endif()
