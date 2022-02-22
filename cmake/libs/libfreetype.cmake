if (TARGET freetype)
    return()
endif()

set( LIB_FREETYPE TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_FREETYPE PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_FREETYPE STREQUAL TryFindPackageFirst)
    find_package(Freetype QUIET)
    if (FREETYPE_FOUND)
        message(STATUS "[LIB_FREETYPE] using system lib.")
        set(LIB_FREETYPE UsingFindPackage)
    else()
        message(STATUS "[LIB_FREETYPE] compiling from source.")
        set(LIB_FREETYPE FromSource)
    endif()
endif()

if(LIB_FREETYPE STREQUAL FromSource)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} freetype)

    tool_include_lib(freetype)

    include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}/freetype/")

elseif(LIB_FREETYPE STREQUAL UsingFindPackage)

    if (NOT TARGET freetype)

        find_package(Freetype REQUIRED QUIET)

        add_library(freetype OBJECT ${FREETYPE_LIBRARIES})
        target_link_libraries(freetype ${FREETYPE_LIBRARIES})
        include_directories(${FREETYPE_INCLUDE_DIRS})
        set_target_properties(freetype PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(freetype PROPERTIES FOLDER "LIBS")

    endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
