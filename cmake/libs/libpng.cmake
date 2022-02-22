if (TARGET libpng)
    return()
endif()

set( LIB_PNG TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_PNG PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_PNG STREQUAL TryFindPackageFirst)
    find_package(PNG QUIET)
    if (PNG_FOUND)
        message(STATUS "[LIB_PNG] using system lib.")
        set(LIB_PNG UsingFindPackage)
    else()
        message(STATUS "[LIB_PNG] compiling from source.")
        set(LIB_PNG FromSource)
    endif()
endif()

if(LIB_PNG STREQUAL FromSource)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} libpng)

    set(SKIP_INSTALL_ALL ON)
    #set(PNG_STATIC ON)
    option(PNG_SHARED "Build shared lib" OFF)
    option(PNG_STATIC "Build static lib" ON)
    option(PNG_TESTS  "Build libpng tests" OFF)
    tool_include_lib(libpng)
    #unset(SKIP_INSTALL_ALL)
    #unset(PNG_STATIC)

    include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}/libpng/")

    add_library(libpng ALIAS png_static)

    #add_library(libpng OBJECT $<TARGET_OBJECTS:png_static>)
    #target_link_libraries(libpng PUBLIC png_static)

    # set the target's folder (for IDEs that support it, e.g. Visual Studio)
    #set_target_properties(libpng PROPERTIES FOLDER "LIBS")

elseif(LIB_PNG STREQUAL UsingFindPackage)

    if (NOT TARGET libpng)

        find_package(PNG REQUIRED QUIET)

        #message("includeDIR: ${PNG_INCLUDE_DIR}")
        #message("Libs: ${PNG_LIBRARIES}")

        add_library(libpng OBJECT ${PNG_LIBRARIES})
        target_link_libraries(libpng ${PNG_LIBRARIES})
        include_directories(${PNG_INCLUDE_DIR})
        set_target_properties(libpng PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(libpng PROPERTIES FOLDER "LIBS")

    endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
