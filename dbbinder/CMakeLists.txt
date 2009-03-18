cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(dbbinder)

link_directories($ENV{ORACLE_HOME}/lib)
include_directories($ENV{ORACLE_HOME}/include)

add_subdirectory(./TinyXML)

add_definitions(-Wall -Wno-parentheses -g -DTIXML_USE_TICPP)

add_custom_command(OUTPUT options.h
					COMMAND gperf
					ARGS -CGD -N isValid -K option -L C++ -t options.gperf > options.h
					DEPENDS options.gperf
					COMMENT build options.h)

find_package( Boost )
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

# MySQL

string(LENGTH "${ENABLE_MYSQL}" ENABLE_MYSQL_LEN)
if (ENABLE_MYSQL_LEN EQUAL 0)
	set(ENABLE_MYSQL ON)
endif(ENABLE_MYSQL_LEN EQUAL 0)

if(ENABLE_MYSQL)
	add_definitions(-DWITH_MYSQL)
	list(APPEND extra_files mysqlgenerator.cpp)
endif(ENABLE_MYSQL)

# Firebird

string(LENGTH "${ENABLE_FIREBIRD}" ENABLE_FIREBIRD_LEN)
if (ENABLE_FIREBIRD_LEN EQUAL 0)
	set(ENABLE_FIREBIRD ON)
endif(ENABLE_FIREBIRD_LEN EQUAL 0)

if(ENABLE_FIREBIRD)
	add_definitions(-DWITH_FIREBIRD)
	list(APPEND extra_files firebirdgenerator.cpp)
endif(ENABLE_FIREBIRD)

# Oracle

string(LENGTH "${ENABLE_ORACLE}" ENABLE_ORACLE_LEN)
if (ENABLE_ORACLE_LEN EQUAL 0)
	set(ENABLE_ORACLE ON)
endif(ENABLE_ORACLE_LEN EQUAL 0)

if(ENABLE_ORACLE)
	add_definitions(-DWITH_ORACLE)
	list(APPEND extra_files oraclegenerator.cpp)
endif(ENABLE_ORACLE)

add_executable(dbbinder main.cpp abstractgenerator sqlitegenerator ${extra_files})

set_source_files_properties(main.cpp PROPERTIES OBJECT_DEPENDS options.h)

target_link_libraries(dbbinder ticpp ctemplate_nothreads sqlite3 ociei mysqlclient fbclient)
