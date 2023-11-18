message(STATUS "Fetching libvterm")
FetchContent_Declare(
	vterm
	GIT_REPOSITORY https://github.com/neovim/libvterm.git
	GIT_TAG v0.3.3
)

FetchContent_GetProperties(vterm)
if(NOT vterm_POPULATED)
    FetchContent_Populate(vterm)

    file(GLOB VTERM_SOURCES ${vterm_SOURCE_DIR}/src/*.c)

    add_library(vterm INTERFACE)

    target_include_directories(vterm INTERFACE
		${vterm_SOURCE_DIR}/src
		${vterm_SOURCE_DIR}/include
	)

    target_compile_options(vterm INTERFACE
        -std=c99
    )

	if (WIN32)
		target_compile_options(vterm INTERFACE
			/W4
		)
	else()
		target_compile_options(vterm INTERFACE
			-Wall
			-Wpedantic
		)
	endif()

    file(GLOB TBLFILES ${vterm_SOURCE_DIR}/src/encoding/*.tbl)
    foreach(TBL_FILE ${TBLFILES})
        get_filename_component(TBL_NAME ${TBL_FILE} NAME_WE)
		set(INC_FILE "${vterm_SOURCE_DIR}/src/encoding/${TBL_NAME}.inc")

        add_custom_command(
            OUTPUT ${INC_FILE}
			COMMAND perl -CSD ${vterm_SOURCE_DIR}/tbl2inc_c.pl ${TBL_FILE} > ${INC_FILE}
            DEPENDS ${TBL_FILE}
            COMMENT "Generating ${INC_FILE}"
        )

        list(APPEND VTERM_SOURCES ${INC_FILE})
    endforeach()

    target_sources(vterm INTERFACE ${VTERM_SOURCES})
endif()
