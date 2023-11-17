message(STATUS "Fetching freetype2")
FetchContent_Declare(
	freetype
	GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
	GIT_TAG VER-2-13-2
)

FetchContent_GetProperties(freetype)
if(NOT freetype_POPULATED)
	FetchContent_Populate(freetype)
	add_subdirectory(${freetype_SOURCE_DIR} ${freetype_BINARY_DIR})
endif()

