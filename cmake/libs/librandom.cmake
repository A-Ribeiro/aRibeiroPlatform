if (TARGET librandom)
    return()
endif()

set( LIB_RANDOM FromSource CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_RANDOM PROPERTY STRINGS None FromSource)

if (LIB_RANDOM STREQUAL FromSource)
    
    message(STATUS "[LIB_RANDOM] compiling from source.")

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} librandom)
    tool_include_lib(librandom)

else()
    message(FATAL_ERROR "You need to select the source of the librandom." )
endif()
