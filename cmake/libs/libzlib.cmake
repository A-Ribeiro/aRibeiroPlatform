if (TARGET zlib)
    return()
endif()

set( LIB_ZLIB TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ZLIB PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_ZLIB STREQUAL TryFindPackageFirst)
    find_package(ZLIB QUIET)
    if (ZLIB_FOUND)
        message(STATUS "[LIB_ZLIB] using system lib.")
        set(LIB_ZLIB UsingFindPackage)
    else()
        message(STATUS "[LIB_ZLIB] compiling from source.")
        set(LIB_ZLIB FromSource)
    endif()
endif()

if(LIB_ZLIB STREQUAL FromSource)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} zlib)

    set(SKIP_INSTALL_ALL ON)
    tool_include_lib(zlib)
    #unset(SKIP_INSTALL_ALL)

    include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}/zlib/")

elseif(LIB_ZLIB STREQUAL UsingFindPackage)

    if (NOT TARGET zlib)

        find_package(ZLIB REQUIRED QUIET)

        add_library(zlib OBJECT ${ZLIB_LIBRARIES})
        target_link_libraries(zlib ${ZLIB_LIBRARIES})
        include_directories(${ZLIB_INCLUDE_DIR})
        set_target_properties(zlib PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(zlib PROPERTIES FOLDER "LIBS")

    endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
