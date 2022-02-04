if (NOT LIB_RANDOM)
    set( LIB_RANDOM FromPackage CACHE STRING "Choose the Library Source." FORCE )
    set_property(CACHE LIB_RANDOM PROPERTY STRINGS None FromPackage FromSource)
endif()

if (LIB_RANDOM STREQUAL FromSource)

    message(FATAL_ERROR "Build from source not implemented yet." )

elseif (LIB_RANDOM STREQUAL FromPackage)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromPackage option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} librandom)
    tool_include_lib(librandom)

else()
    message(FATAL_ERROR "You need to select the source of the librandom." )
endif()
