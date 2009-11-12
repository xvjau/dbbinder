find_program(DBBINDER_EXECUTABLE NAMES dbbinder)

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

include(AddFileDependencies)

macro(generate_sql_bindings sql_files)
	foreach(file ${ARGV})
		get_filename_component(SQLFILE ${file} ABSOLUTE)
		get_filename_component(SQLFILE_WE ${file} NAME_WE)
		get_filename_component(SQLFILEPATH ${SQLFILE} PATH)

		#set(DBBINDER_OUTPUT_PATH ${SQLFILEPATH}/.tmp/dbbinder/)
		#execute_process(COMMAND mkdir -p ${DBBINDER_OUTPUT_PATH})

		set(DBBINDER_OUTPUT_PATH ${SQLFILEPATH})

		include_directories(${DBBINDER_OUTPUT_PATH})

		#get_directory_property(clean_file_list ADDITIONAL_MAKE_CLEAN_FILES)
		#set(clean_file_list "${clean_file_list};${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.h${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.cpp")
		#set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${clean_file_list})

		list(APPEND bound_cpp_files ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.cpp)

		add_custom_command(OUTPUT "${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE}.cpp"
							COMMAND ${DBBINDER_EXECUTABLE}
							ARGS -i ${SQLFILE} -o ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE} ${DBBINDER_EXTRA_ARGS}
							DEPENDS ${SQLFILE}
							WORKING_DIRECTORY ${DBBINDER_OUTPUT_PATH}
							COMMENT dbbinder ${SQLFILE})

		execute_process(COMMAND ${DBBINDER_EXECUTABLE} --depends -i ${SQLFILE} -o ${DBBINDER_OUTPUT_PATH}/${SQLFILE_WE} ${DBBINDER_EXTRA_ARGS}
						WORKING_DIRECTORY ${DBBINDER_OUTPUT_PATH}
						RESULT_VARIABLE DEPENDS_RES
						OUTPUT_VARIABLE DEPENDS
						OUTPUT_STRIP_TRAILING_WHITESPACE)

		if (${DEPENDS_RES} EQUAL 0)
			string(REPLACE "\n" ";" DEPENDS ${DEPENDS})

			list(FIND DEPENDS "Depends:" pos)
			math(EXPR pos "${pos} + 1")

			while(${pos} GREATER -1)
				list(REMOVE_AT DEPENDS ${pos})
				math(EXPR pos "${pos} - 1")
			endwhile()

			foreach(FILE ${DEPENDS})
				ADD_FILE_DEPENDENCIES("${SQLFILE}" "${FILE}")
			endforeach()
		endif()

	endforeach()
endmacro()
