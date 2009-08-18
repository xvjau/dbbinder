cmake_minimum_required(VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(example_sqlite)

get_source_file_property(SQLFILE test.sql LOCATION)

add_custom_target(test.db
					sqlite3 -init ${SQLFILE} test.db ".quit"
					COMMENT sqlite3 test.sql
					SOURCES test.sql
					)

get_source_file_property(SQLFILE test_select.sql LOCATION)
get_filename_component(SQLFILEPATH ${SQLFILE} PATH)

add_custom_command(OUTPUT ${SQLFILEPATH}/test_select.cpp
					COMMAND ${SQLFILEPATH}/../../../dbbinder
					ARGS -i ${SQLFILE} -o ${SQLFILEPATH}/test_select -d ${SQLFILEPATH}/../../../templates/
					DEPENDS ${SQLFILE}
					COMMENT dbbinder ${SQLFILE})

add_executable(example_sqlite  main.cpp test_select.cpp)

add_dependencies(example_sqlite test.db)

target_link_libraries(example_sqlite sqlite3)

