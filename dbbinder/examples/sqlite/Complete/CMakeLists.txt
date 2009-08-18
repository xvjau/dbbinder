cmake_minimum_required(VERSION 2.6)

project(example_sqlite)

get_source_file_property(SQLFILE test_select.sql LOCATION)
get_filename_component(SQLFILEPATH ${SQLFILE} PATH)

add_custom_command(OUTPUT test_select.cpp
					COMMAND ${SQLFILEPATH}/../../../dbbinder
					ARGS -i ${SQLFILE}
					DEPENDS ${SQLFILE}
					COMMENT dbbinder ${SQLFILE})

add_executable(example_sqlite  main.cpp test_select.cpp)


