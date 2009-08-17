cmake_minimum_required(VERSION 2.6)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif()

set (DEBUG ON)
if (DEBUG)
	set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

project(dbbinder)

add_definitions(-fvisibility=hidden -Wall -Wno-parentheses)

add_subdirectory(./TinyXML)

add_definitions(-DTIXML_USE_TICPP)

########################## Gnu Perfect Hash ##########################

get_filename_component(SRC_PATH ./options.gperf ABSOLUTE)
get_filename_component(DST_PATH ./options.h ABSOLUTE)

add_custom_command(OUTPUT options.h
					COMMAND gperf
					ARGS -CGD -N isValid -K option -L C++ -t ${SRC_PATH} > ${DST_PATH}
					DEPENDS options.gperf
					COMMENT build options.h)

set_source_files_properties(main.cpp PROPERTIES OBJECT_DEPENDS options.h)

########################## Boost ##########################

find_package( Boost )
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

##########################  MySQL ##########################

string(LENGTH "${ENABLE_MYSQL}" ENABLE_MYSQL_LEN)
if (ENABLE_MYSQL_LEN EQUAL 0)
	set(ENABLE_MYSQL ON)
endif()

if(ENABLE_MYSQL)
	find_file(MYSQL_INCLUDE mysql/mysql.h)
	find_library(MYSQL_LIB mysqlclient)
	if (MYSQL_INCLUDE AND MYSQL_LIB)
		message(STATUS "MySQL found.")
		add_definitions(-DWITH_MYSQL)

		list(APPEND extra_files mysqlgenerator.cpp)
		get_filename_component(MYSQL_INCLUDE_PATH ${MYSQL_INCLUDE} PATH)
		include_directories(${MYSQL_INCLUDE_PATH})

		list(APPEND extra_libs mysqlclient)
		get_filename_component(MYSQL_LIB_PATH ${MYSQL_LIB} PATH)
		link_directories(${MYSQL_LIB_PATH})

	else(MYSQL_INCLUDE AND MYSQL_LIB)
		message(STATUS "MySQL not found.")
	endif(MYSQL_INCLUDE AND MYSQL_LIB)
endif()

########################## Firebird ##########################

string(LENGTH "${ENABLE_FIREBIRD}" ENABLE_FIREBIRD_LEN)
if (ENABLE_FIREBIRD_LEN EQUAL 0)
	set(ENABLE_FIREBIRD ON)
endif()

if(ENABLE_FIREBIRD)
	find_file(FIREBIRD_CONFIG fb_config PATH /usr/bin /usr/local/bin /opt/firebird/bin)
	if (FIREBIRD_CONFIG)
		execute_process(COMMAND ${FIREBIRD_CONFIG} --version
					OUTPUT_VARIABLE FIREBIRD_VERSION)
		string(REGEX REPLACE "\n" "" FIREBIRD_VERSION ${FIREBIRD_VERSION})
		message(STATUS "Firebird found version ${FIREBIRD_VERSION}")

		add_definitions(-DWITH_FIREBIRD)
		list(APPEND extra_files firebirdgenerator.cpp)

		execute_process(COMMAND ${FIREBIRD_CONFIG} --cflags
						OUTPUT_VARIABLE FIREBIRD_CFLAGS)
		string(REGEX REPLACE "\n" "" FIREBIRD_CFLAGS ${FIREBIRD_CFLAGS})
		add_definitions(${FIREBIRD_CFLAGS})

		execute_process(COMMAND ${FIREBIRD_CONFIG} --libs
						OUTPUT_VARIABLE FIREBIRD_LINKFLAGS)
		string(REGEX REPLACE "\n" "" FIREBIRD_LINKFLAGS ${FIREBIRD_LINKFLAGS})

		string(REGEX MATCHALL "-L([^ ]+)" FIREBIRD_LIBS ${FIREBIRD_LINKFLAGS})
		string(REGEX REPLACE "-L" "" FIREBIRD_LIBS ${FIREBIRD_LIBS})
		link_directories(${FIREBIRD_LIBS})

		string(REGEX MATCHALL "-l([^ ]+)" FIREBIRD_LIBS ${FIREBIRD_LINKFLAGS})
		string(REGEX REPLACE "-l" "" FIREBIRD_LIBS ${FIREBIRD_LIBS})
		list(APPEND extra_libs ${FIREBIRD_LIBS})

	else(FIREBIRD_CONFIG)
		find_file(FIREBIRD_INCLUDE ibase.h PATH /usr/include /usr/local/include)

		find_library(FIREBIRD_LIB fbclient)
		set(FIREBIRD_LIB_NAME fbclient)

		if (NOT FIREBIRD_LIB)
			find_library(FIREBIRD_LIB gds)
			set(FIREBIRD_LIB_NAME gds)
		endif ()

		if (FIREBIRD_INCLUDE AND FIREBIRD_LIB)
			message(STATUS "Firebird found.")
			add_definitions(-DWITH_FIREBIRD)

			list(APPEND extra_files firebirdgenerator.cpp)
			get_filename_component(FIREBIRD_INCLUDE_PATH ${FIREBIRD_INCLUDE} PATH)
			include_directories(${FIREBIRD_INCLUDE_PATH})

			list(APPEND extra_libs ${FIREBIRD_LIB_NAME})
			get_filename_component(FIREBIRD_LIB_PATH ${FIREBIRD_LIB} PATH)
			link_directories(${FIREBIRD_LIB_PATH})

		else(FIREBIRD_INCLUDE AND FIREBIRD_LIB)
			message(STATUS "Firebird not found.")
		endif()
	endif()
endif()

########################## Oracle ##########################

string(LENGTH "${ENABLE_ORACLE}" ENABLE_ORACLE_LEN)
if (ENABLE_ORACLE_LEN EQUAL 0)
	set(ENABLE_ORACLE ON)
endif()

if(ENABLE_ORACLE)
	find_file(ORACLE_INCLUDE oci.h PATHS $ENV{C_INCLUDE_PATH})
	find_library(ORACLE_LIB ociei PATHS $ENV{ORACLE_HOME}/lib)

	if (ORACLE_INCLUDE AND ORACLE_LIB)
		message(STATUS "Oracle found.")
		add_definitions(-DWITH_ORACLE)

		list(APPEND extra_files oraclegenerator.cpp)
		get_filename_component(ORACLE_INCLUDE_PATH ${ORACLE_INCLUDE} PATH)
		include_directories(${ORACLE_INCLUDE_PATH})

		list(APPEND extra_libs ociei)
		get_filename_component(ORACLE_LIB_PATH ${ORACLE_LIB} PATH)
		link_directories(${ORACLE_LIB_PATH})

	else (ORACLE_INCLUDE AND ORACLE_LIB)
		message(STATUS "Oracle not found.")
	endif ()
endif()

########################## YAML ##########################

string(LENGTH "${ENABLE_YAML}" ENABLE_YAML_LEN)
if (ENABLE_YAML_LEN EQUAL 0)
	set(ENABLE_YAML ON)
endif()

if(ENABLE_YAML)
	find_file(YAML_INCLUDE yaml.h)
	find_library(YAML_LIB yaml)

	if (YAML_INCLUDE AND YAML_LIB)
		message(STATUS "YAML found.")
		add_definitions(-DWITH_YAML)

		list(APPEND extra_files yaml_reader.cpp)
		get_filename_component(YAML_INCLUDE_PATH ${YAML_INCLUDE} PATH)
		include_directories(${YAML_INCLUDE_PATH})

		list(APPEND extra_libs yaml)
		get_filename_component(YAML_LIB_PATH ${YAML_LIB} PATH)
		link_directories(${YAML_LIB_PATH})

	else (YAML_INCLUDE AND YAML_LIB)
		message(STATUS "YAML not found.")
	endif ()
endif()

########################## Done ##########################

add_executable(dbbinder main.cpp xml_reader.cpp abstractgenerator.cpp sqlitegenerator.cpp ${extra_files})

target_link_libraries(dbbinder ticpp ctemplate_nothreads sqlite3 ${extra_libs})

