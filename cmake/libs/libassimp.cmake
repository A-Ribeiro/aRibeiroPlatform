if (TARGET assimp)
    return()
endif()

set( LIB_ASSIMP TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ASSIMP PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_ASSIMP STREQUAL TryFindPackageFirst)
    #find_package(assimp QUIET)
    find_path(
        assimp_INCLUDE_DIRS
        NAMES assimp/postprocess.h assimp/scene.h assimp/version.h assimp/config.h assimp/cimport.h
        PATHS /usr/local/include
        PATHS /usr/include/
    )

    find_library(
        assimp_LIBRARIES
        NAMES assimp
        PATHS /usr/local/lib/
        PATHS /usr/lib64/
        PATHS /usr/lib/
    )

    if (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
	  set(assimp_FOUND TRUE)
    else()
        unset(assimp_INCLUDE_DIRS CACHE)
        unset(assimp_LIBRARIES CACHE)
	endif()

    if (assimp_FOUND)
        message(STATUS "[LIB_ASSIMP] using system lib.")
        set(LIB_ASSIMP UsingFindPackage)
    else()
        message(STATUS "[LIB_ASSIMP] compiling from source.")
        set(LIB_ASSIMP FromSource)
    endif()
endif()

if(LIB_ASSIMP STREQUAL FromSource)

    # if (NOT LIBS_REPOSITORY_URL)
    #     message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    # endif()
    # tool_download_lib_package(${LIBS_REPOSITORY_URL} freetype)
    # tool_include_lib(freetype)
    # include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}/freetype/" PARENT_SCOPE)


    tool_download_git_package("https://github.com/assimp/assimp.git" assimp)


    OPTION ( ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." OFF)
    OPTION( ASSIMP_BUILD_ASSIMP_TOOLS "If the supplementary tools for Assimp are built in addition to the library." OFF )
    OPTION( ASSIMP_INSTALL "Disable this if you want to use assimp as a submodule." OFF)
    #avoid link problem
    unset(ZLIB_FOUND)

    tool_include_lib(assimp)

    include_directories("${ARIBEIRO_LIBS_DIR}/${LIBNAME}/include/")

    # set the target's folder (for IDEs that support it, e.g. Visual Studio)
    if (TARGET assimp)
        set_target_properties(assimp PROPERTIES FOLDER "LIBS/assimp")
    endif()
    if (TARGET zlibstatic)
        set_target_properties(zlibstatic PROPERTIES FOLDER "LIBS/assimp")
    endif()
    if (TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
        set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "LIBS/assimp")
    endif()
    
    

    #message(FATAL_ERROR "Compiling from Source not implemented")

elseif(LIB_ASSIMP STREQUAL UsingFindPackage)

    if (NOT TARGET assimp)
        
        #find_package(assimp REQUIRED QUIET)

        # find_path(
        #     assimp_INCLUDE_DIRS
        #     NAMES assimp/postprocess.h assimp/scene.h assimp/version.h assimp/config.h assimp/cimport.h
        #     PATHS /usr/local/include
        #     PATHS /usr/include/
        # )

        # find_library(
        #     assimp_LIBRARIES
        #     NAMES assimp
        #     PATHS /usr/local/lib/
        #     PATHS /usr/lib64/
        #     PATHS /usr/lib/
        # )

        add_library(assimp OBJECT ${assimp_LIBRARIES})
        target_link_libraries(assimp ${assimp_LIBRARIES})
        include_directories(${assimp_INCLUDE_DIRS})
        set_target_properties(assimp PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(assimp PROPERTIES FOLDER "LIBS")

    endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
