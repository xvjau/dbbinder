FIND_PROGRAM(DBBINDER_EXECUTABLE NAMES dbbinder 
				PATHS /Users/gianni/Projects/workspace/dbbinder/)

if (NOT DBBINDER_EXECUTABLE)
	message(FATAL_ERROR "dbbinder executable not found")
endif()

#Need to add these:
#XXX_VERSION_PATCH       The patch version of the package found, if any.
#XXX_INCLUDE_DIRS        The final set of include directories listed in one variable for use by client code.  This should not be a cache entry.

execute_process(COMMAND ${DBBINDER_EXECUTABLE} -V
				  OUTPUT_VARIABLE DBBINDER_VERSION_STRING
				  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${DBBINDER_EXECUTABLE} --vmajor
				  OUTPUT_VARIABLE DBBINDER_VERSION_MAJOR
				  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${DBBINDER_EXECUTABLE} --vminor
				  OUTPUT_VARIABLE DBBINDER_VERSION_MINOR
				  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (${UNIX})
	set(DBBINDER_OUTPUT_PATH ${CMAKE_CACHEFILE_DIR}/CMakeFiles/CMakeTmp/dbbinder/)
	execute_process(COMMAND mkdir "${DBBINDER_OUTPUT_PATH}")
else()
	message(STATUS "annot create temp directory")
	set(DBBINDER_OUTPUT_PATH ${CMAKE_CACHEFILE_DIR}/CMakeFiles/CMakeTmp/)
endif()

include_directories(${DBBINDER_OUTPUT_PATH})
message(STATUS "Dir: ${DBBINDER_OUTPUT_PATH}")

macro(generate_sql_bindings sql_files)
	foreach(file ${sql_files})
	
		get_filename_component(SQLFILE ${file} ABSOLUTE)
		get_filename_component(SQLFILE_WE ${file} NAME_WE)
		get_filename_component(SQLFILEPATH ${SQLFILE} PATH)
		
		list(APPEND bound_cpp_files ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.cpp)
		
		set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.h)
		
		add_custom_command(OUTPUT ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.cpp
							COMMAND ${DBBINDER_EXECUTABLE}
							ARGS -i ${SQLFILE} -o ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}
							DEPENDS ${SQLFILE}
							COMMENT dbbinder ${SQLFILE})
	endforeach()
endmacro() 
