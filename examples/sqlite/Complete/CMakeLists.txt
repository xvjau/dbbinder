cmake_minimum_required(VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(example_sqlite)

find_package(DBBinder REQUIRED)

message(STATUS ${DBBINDER_EXECUTABLE})

find_file(TEST_DB test.db PATHS ${CMAKE_CURRENT_LIST_DIR})
if(NOT TEST_DB)
	execute_process(COMMAND sqlite3 -init /home/gianni/Projects/dbbinder/examples/sqlite/Complete/test.sql ${CMAKE_CURRENT_LIST_DIR}/test.db ".exit" RESULT_VARIABLE TEST_DB OUTPUT_QUIET)
	find_file(TEST_DB test.db PATHS ${CMAKE_CURRENT_LIST_DIR})
	if(NOT TEST_DB)
		message(FATAL_ERROR "Test DB not found")
	endif()
endif()

generate_sql_bindings(test_select.sql)

add_executable(example_sqlite main.cpp db.cpp ${bound_cpp_files})

get_source_file_property(SQLFILEPATH test.sql PATH)

# add_custom_command(TARGET example_sqlite
# 					PRE_BUILD
# 					COMMAND sqlite3 -init test.sql test.db ".quit"
# 					WORKING_DIRECTORY ${SQLFILEPATH}
# 					COMMENT sqlite3 test.sql
# 					)

target_link_libraries(example_sqlite sqlite3)

