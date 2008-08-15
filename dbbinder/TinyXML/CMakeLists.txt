add_definitions(-DTIXML_USE_TICPP)

add_library(ticpp STATIC
	ticpp.cpp
	tinystr.cpp
	tinyxml.cpp
	tinyxmlerror.cpp
	tinyxmlparser.cpp)

#remove a 'gordura'
if(NOT debug AND NOT ${APPLE})
	get_target_property(filename ${target_name} LOCATION)
	add_custom_command(TARGET ${target_name} POST_BUILD COMMAND ${platform}-strip ARGS ${strip_args} ${filename} COMMENT stripping ${target_name})
endif(NOT debug AND NOT ${APPLE})
