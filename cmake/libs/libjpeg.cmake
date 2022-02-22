if (TARGET libjpeg)
    return()
endif()

set( LIB_JPEG TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_JPEG PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_JPEG STREQUAL TryFindPackageFirst)
    find_package(JPEG QUIET)
    if (JPEG_FOUND)
        message(STATUS "[LIB_JPEG] using system lib.")
        set(LIB_JPEG UsingFindPackage)
    else()
        message(STATUS "[LIB_JPEG] compiling from source.")
        set(LIB_JPEG FromSource)
    endif()
endif()

if(LIB_JPEG STREQUAL FromSource)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} libjpeg)

    tool_include_lib(libjpeg)

    include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}/libjpeg/")

elseif(LIB_JPEG STREQUAL UsingFindPackage)

    if (NOT TARGET libjpeg)

        find_package(JPEG REQUIRED QUIET)

        #message("includeDIR: ${JPEG_INCLUDE_DIR}")
        #message("Libs: ${JPEG_LIBRARIES}")

        add_library(libjpeg OBJECT ${JPEG_LIBRARIES})
        target_link_libraries(libjpeg ${JPEG_LIBRARIES})
        include_directories(${JPEG_INCLUDE_DIR})
        set_target_properties(libjpeg PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(libjpeg PROPERTIES FOLDER "LIBS")

    endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
