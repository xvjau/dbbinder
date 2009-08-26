cmake_minimum_required(VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(example_sqlite)


find_package(DBBinder REQUIRED)

message(STATUS ${DBBINDER_EXECUTABLE})

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

