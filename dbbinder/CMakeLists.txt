cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(dbbinder)

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
	find_file(MYSQL_INCLUDE mysql/mysql.h)
	find_file(MYSQL_LIB libmysqlclient.so PATHS /usr/lib /usr/lib/mysql)
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
endif(ENABLE_MYSQL)

# Firebird

string(LENGTH "${ENABLE_FIREBIRD}" ENABLE_FIREBIRD_LEN)
if (ENABLE_FIREBIRD_LEN EQUAL 0)
	set(ENABLE_FIREBIRD ON)
endif(ENABLE_FIREBIRD_LEN EQUAL 0)

if(ENABLE_FIREBIRD)
	find_file(FIREBIRD_INCLUDE ibase.h PATH /usr/include /usr/local/include)
	
	find_file(FIREBIRD_LIB libfbclient.so PATHS /usr/lib /usr/local/lib)
	set(FIREBIRD_LIB_NAME fbclient)
	
	if (NOT FIREBIRD_LIB)
		find_file(FIREBIRD_LIB libgds.so PATHS /usr/lib /usr/local/lib)
		set(FIREBIRD_LIB_NAME gds)
	endif (NOT FIREBIRD_LIB)
	
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
	endif(FIREBIRD_INCLUDE AND FIREBIRD_LIB)
endif(ENABLE_FIREBIRD)

# Oracle

string(LENGTH "${ENABLE_ORACLE}" ENABLE_ORACLE_LEN)
if (ENABLE_ORACLE_LEN EQUAL 0)
	set(ENABLE_ORACLE ON)
endif(ENABLE_ORACLE_LEN EQUAL 0)

if(ENABLE_ORACLE)
	find_file(ORACLE_INCLUDE oci.h PATHS $ENV{C_INCLUDE_PATH})
	find_file(ORACLE_LIB libociei.so PATHS $ENV{ORACLE_HOME}/lib)

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
	endif (ORACLE_INCLUDE AND ORACLE_LIB)
endif(ENABLE_ORACLE)

add_executable(dbbinder main.cpp abstractgenerator sqlitegenerator ${extra_files})

set_source_files_properties(main.cpp PROPERTIES OBJECT_DEPENDS options.h)

target_link_libraries(dbbinder ticpp ctemplate_nothreads sqlite3 ${extra_libs})
