project(TinyXMLParser)

set(_TinyXMLParser_CEGUI_INCLUDE_DIR "${TinyXMLParser_SOURCE_DIR}/../../include")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${TinyXMLParser_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the TinyXMLParser. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES CEGUITinyXMLParser.cpp
            CEGUITinyXMLParserModule.cpp
            ceguitinyxml/tinystr.cpp
            ceguitinyxml/tinyxml.cpp
            ceguitinyxml/tinyxmlerror.cpp
            ceguitinyxml/tinyxmlparser.cpp
   )

add_library(CEGUITinyXMLParser ${SOURCES})
target_compile_definitions(CEGUITinyXMLParser PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)

# Add additional include search directories.
target_include_directories(CEGUITinyXMLParser PRIVATE ${_TinyXMLParser_CEGUI_INCLUDE_DIR})
target_include_directories(CEGUITinyXMLParser PUBLIC ${TinyXMLParser_SOURCE_DIR})

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(CEGUITinyXMLParser PRIVATE -Wall -fpermissive)
    target_compile_options(CEGUITinyXMLParser PRIVATE $<$<CONFIG:Debug>:-pg>)
endif()
