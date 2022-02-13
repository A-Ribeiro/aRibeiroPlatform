cmake_minimum_required (VERSION 3.5.1)

project(aRibeiroPlatform)

# "For this is how God loved the world:
# he gave his only Son, so that everyone
# who believes in him may not perish
# but may have eternal life."
#
# John 3:16

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

ENABLE_LANGUAGE(CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake-modules)

#the path we need to get the custom lib versions
set(LIBS_REPOSITORY_URL "https://github.com/A-Ribeiro/public_libs/raw/main")

include(cmake/include.cmake)
include(cmake/libs/libaRibeiroCore.cmake)

FILE( GLOB_RECURSE PUBLIC_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.h")
FILE( GLOB_RECURSE SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

copy_headers_to_include_directory( ${PROJECT_NAME} ${PUBLIC_HEADERS} )
define_source_group_base_path("src" ${PUBLIC_HEADERS} ${SRC})

configure_include_file( ${PROJECT_NAME} "src/${PROJECT_NAME}.h.in" "${PROJECT_NAME}.h" ${PUBLIC_HEADERS} )

tool_get_directory_definitions(compile_defs)
get_directory_property(compile_opts COMPILE_OPTIONS)

add_library( ${PROJECT_NAME} STATIC ${PUBLIC_HEADERS} ${SRC} )

set(TARGET_LIBS "")
if (ARIBEIRO_RPI)
    list(APPEND TARGET_LIBS ${RPI_LIBRARIES} )
endif()
if(OS_TARGET STREQUAL win)
    list(APPEND TARGET_LIBS
                shlwapi iphlpapi #path operations
                winmm #multimedia calls
                ws2_32 #WinSock2
    )
elseif(OS_TARGET STREQUAL mac)
elseif(OS_TARGET STREQUAL linux)
    list(APPEND TARGET_LIBS pthread rt )
else()
    message(FATAL_ERROR "Platform not tested error")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC aRibeiroCore ${TARGET_LIBS})

# set the target's folder (for IDEs that support it, e.g. Visual Studio)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "aRibeiro")

target_compile_definitions(${PROJECT_NAME} PUBLIC ${compile_defs})
target_compile_options(${PROJECT_NAME} PUBLIC ${compile_opts})

option(ARIBEIRO_SKIP_INSTALL_PLATFORM OFF)

if( NOT MSVC AND NOT ARIBEIRO_SKIP_INSTALL_PLATFORM )
    # Install setup
    install(
        TARGETS ${PROJECT_NAME} 
        ARCHIVE DESTINATION lib
    )
    install(
        FILES 
            ${PUBLIC_HEADERS} 
            "${CMAKE_HOME_DIRECTORY}/include/${PROJECT_NAME}/${PROJECT_NAME}.h"
            "LICENSE"
        DESTINATION include/${PROJECT_NAME}
    )
endif()
if (NOT supress_show_info)
    message(STATUS "${PROJECT_NAME}")
    list_to_string("${compile_defs}" compile_defs_str)
    message(STATUS "Compile Definitions: ${compile_defs_str}")
    list_to_string("${compile_opts}" compile_opts_str)
    message(STATUS "Compile Options: ${compile_opts_str}")
endif()

