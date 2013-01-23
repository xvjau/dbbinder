cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(COMPILE_DEFINITIONS "")
    set(COMPILE_DEFINITIONS_DEBUG "-ggdb -pipe")
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

project(dbbinder)

add_definitions(-fvisibility=hidden -Wall -Wno-parentheses)

if(APPLE)
    add_definitions(-m64)
endif()

add_subdirectory(./TinyXML)

add_definitions(-DTIXML_USE_TICPP)

########################## Boost ##########################

find_package( Boost COMPONENTS program_options REQUIRED )
include_directories(${Boost_INCLUDE_DIRS})

##########################  google-ctemplate ##########################


find_file( CTEMPLATE_INC "ctemplate/template.h" )
if( NOT CTEMPLATE_INC )
    find_file( CTEMPLATE_INC "google/template.h" )
endif()

if( NOT CTEMPLATE_INC )
    message(FATAL_ERROR "google-ctemplate (includes) not found!")
else()
    get_filename_component( CTEMPLATE_INC ${CTEMPLATE_INC} PATH )
    message(STATUS "Found ctemplate in: ${CTEMPLATE_INC}")
    include_directories(${CTEMPLATE_INC})
endif()

find_library( CTEMPLATE_LIB ctemplate_nothreads )

if(CTEMPLATE_LIB-NOTFOUND)
    find_library( CTEMPLATE_LIB ctemplate )
endif()

if(CTEMPLATE_LIB-NOTFOUND)
    message(FATAL_ERROR "google-ctemplate (lib) not found!")
endif()

##########################  MySQL ##########################

string(LENGTH "${ENABLE_MYSQL}" ENABLE_MYSQL_LEN)
if (ENABLE_MYSQL_LEN EQUAL 0)
    set(ENABLE_MYSQL ON)
endif()

if(ENABLE_MYSQL)
    execute_process(COMMAND mysql_config --version OUTPUT_VARIABLE str_version OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND mysql_config --libs RESULT_VARIABLE RET OUTPUT_VARIABLE str_libs OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${RET} EQUAL 0)

        execute_process(COMMAND mysql_config --cflags RESULT_VARIABLE RET OUTPUT_VARIABLE str_includes OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (${RET} EQUAL 0)
            message(STATUS "MySQL found version ${str_version}")
            add_definitions(-DWITH_MYSQL)
            list(APPEND extra_files mysqlgenerator.cpp)

            string(FIND ${str_libs} " " pos)
            if(${pos} EQUAL -1)
                string(LENGTH ${str_libs} pos)
            endif()

            while(${pos} GREATER 0)
                math(EXPR length "${pos} - 1")
                math(EXPR next "${pos} + 1")
                string(LENGTH ${str_libs} strlength)
                math(EXPR strlength "${strlength} - 1")

                string(SUBSTRING ${str_libs} 1 1 prefix)

                if (length GREATER strlength OR length EQUAL strlength)
                    string(SUBSTRING ${str_libs} 2 -1 value)
                    set(pos -1)
                else()
                    string(SUBSTRING ${str_libs} 2 ${length} value)

                    string(SUBSTRING ${str_libs} ${next} -1 str_libs)
                    string(FIND ${str_libs} " " pos)
                endif()

                string(STRIP ${value} value)

                if (${prefix} STREQUAL "L")
                    link_directories(${value})
                elseif(${prefix} STREQUAL "l")
                    list(APPEND extra_libs ${value})
                endif()
            endwhile()

            string(FIND ${str_includes} " " pos)
            if(${pos} EQUAL -1)
                string(LENGTH ${str_includes} pos)
            endif()

            while(${pos} GREATER 0)
                math(EXPR length "${pos} - 1")
                math(EXPR next "${pos} + 1")
                string(LENGTH ${str_includes} strlength)
                math(EXPR strlength "${strlength} - 1")

                string(SUBSTRING ${str_includes} 1 1 prefix)

                if (length GREATER strlength OR length EQUAL strlength)
                    string(SUBSTRING ${str_includes} 2 -1 value)
                    set(pos -1)
                else()
                    string(SUBSTRING ${str_includes} 2 ${length} value)

                    string(SUBSTRING ${str_includes} ${next} -1 str_includes)
                    string(FIND ${str_includes} " " pos)
                endif()

                string(STRIP ${value} value)

                if (${prefix} STREQUAL "I")
                    include_directories(${value})
                endif()
            endwhile()
        endif()
    endif()
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
        find_file(FIREBIRD_INCLUDE ibase.h PATH /usr/include /usr/local/include /Library/Frameworks/Firebird.framework/)

        find_library(FIREBIRD_LIB fbclient PATHS /Library/Frameworks/Firebird.framework/Libraries)
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

message(STATUS ${Boost_LIBRARIES})

add_executable(dbbinder
    main.cpp
    xml_reader.cpp
    sql_reader.cpp
    abstractgenerator.cpp
    sqlitegenerator.cpp
    ${extra_files})

target_link_libraries(dbbinder ticpp ${CTEMPLATE_LIB} sqlite3 ${extra_libs} ${Boost_LIBRARIES})

install(TARGETS dbbinder RUNTIME DESTINATION bin)

install(DIRECTORY templates
            DESTINATION share/dbbinder/
            PATTERN ".svn" EXCLUDE
            PATTERN "*~" EXCLUDE
            PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(FILES extra/CMake/DBBinderConfig.cmake
        DESTINATION share/dbbinder/
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(FILES extra/blobstream.h
        DESTINATION include
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

install(CODE "execute_process(COMMAND install --mode=a+r,a-wx ${CMAKE_CURRENT_LIST_DIR}/extra/CMake/DBBinderConfig.cmake ${CMAKE_ROOT}/Modules)")
