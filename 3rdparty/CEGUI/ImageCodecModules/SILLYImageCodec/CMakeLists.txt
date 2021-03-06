# Bla

project(CEGUISILLYImageCodec)

set(_CEGUISILLYImageCodec_SILLY_INCLUDE_DIR "${CEGUISILLYImageCodec_SOURCE_DIR}/../../../silly/include")
set(_CEGUISILLYImageCodec_CEGUI_INCLUDE_DIR "${CEGUISILLYImageCodec_SOURCE_DIR}/../../include")

# Setup the make environement. #
################################

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${CEGUISILLYImageCodec_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the CEGUISILLYImageCodec. Please go into the \"build\" directory and type \"cmake ..\" there.\nThank you.")
endif()

# Search for the different needed libs. #
#########################################

# The std:: namespace.
include(${CMAKE_ROOT}/Modules/TestForSTDNamespace.cmake)

if(${CMAKE_NO_STD_NAMESPACE})
    message(SEND_ERROR "No std:: Namespace is available, This usually means you have no c++ support or no libstdc++")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES CEGUISILLYImageCodec.cpp
            CEGUISILLYImageCodecModule.cpp
   )

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    add_definitions(-DDEBUG=1)
else()
    message(STATUS "Configuring for release")
endif()

# Add additional include search directories.
include_directories(${CEGUISILLYImageCodec_SOURCE_DIR}
                    ${_CEGUISILLYImageCodec_SILLY_INCLUDE_DIR}
                    ${_CEGUISILLYImageCodec_CEGUI_INCLUDE_DIR}
                   )

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
elseif(MSVC)
endif()

# And show the flags.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CXX Flags for the CEGUISILLYImageCodec (Debug) are: ${CMAKE_CXX_FLAGS_DEBUG}")
    set(OUT_NAME "CEGUISILLYImageCodecd")
else()
    message(STATUS "CXX Flags for the CEGUISILLYImageCodec (Release) are: ${CMAKE_CXX_FLAGS_RELEASE}")
    set(OUT_NAME "CEGUISILLYImageCodec")
endif()

# The compiling process. #
##########################
add_library(${OUT_NAME} ${SOURCES})
# target_link_libraries(fts ${FTS_LIBS})

