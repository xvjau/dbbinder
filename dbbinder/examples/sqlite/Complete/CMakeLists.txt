cmake_minimum_required(VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(example_sqlite)

get_source_file_property(SQLFILE test.sql LOCATION)

add_custom_target(test.db
					sqlite3 -init ${SQLFILE} test.db ".quit"
					COMMENT sqlite3 test.sql
					SOURCES test.sql
					)

find_package(DBBinder REQUIRED)

generate_sql_bindings(test_select.sql)

add_executable(example_sqlite main.cpp ${bound_cpp_files})

add_dependencies(example_sqlite test.db)

target_link_libraries(example_sqlite sqlite3)

