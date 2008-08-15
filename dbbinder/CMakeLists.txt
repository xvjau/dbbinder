cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(dbbinder)

add_subdirectory(./TinyXML)

add_definitions(-Wall -Wno-parentheses -g -DTIXML_USE_TICPP)

add_custom_command(OUTPUT options.h COMMAND gperf ARGS -CGD -N isValid -K option -L C++ -t options.gperf > options.h DEPENDS options.gperf COMMENT build options.h)

add_executable(dbbinder main.cpp abstractgenerator sqlitegenerator)

set_source_files_properties(main.cpp PROPERTIES OBJECT_DEPENDS options.h)

target_link_libraries(dbbinder ticpp ctemplate_nothreads sqlite3)