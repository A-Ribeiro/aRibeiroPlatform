set( LIB_SFML TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_SFML PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromSource)

if(LIB_SFML STREQUAL TryFindPackageFirst)
    find_package(SFML QUIET)

    if (SFML_INCLUDE_DIRS AND SFML_LIBRARIES)
	  set(SFML_FOUND TRUE)
    else()
        unset(SFML_INCLUDE_DIRS CACHE)
        unset(SFML_LIBRARIES CACHE)
	endif()

    if (SFML_FOUND)
        message(STATUS "[LIB_SFML] using system lib.")
        set(LIB_SFML UsingFindPackage)
    else()
        message(STATUS "[LIB_SFML] compiling from source.")
        set(LIB_SFML FromSource)
    endif()
endif()

if(LIB_SFML STREQUAL FromSource)

    tool_download_git_package("https://github.com/SFML/SFML.git" sfml)

    set(BUILD_SHARED_LIBS OFF)

    set(SFML_BUILD_WINDOW TRUE CACHE BOOL "TRUE to build SFML's Window module. This setting is ignored, if the graphics module is built.")
    set(SFML_BUILD_GRAPHICS TRUE CACHE BOOL "TRUE to build SFML's Graphics module.")
    set(SFML_BUILD_AUDIO TRUE CACHE BOOL "TRUE to build SFML's Audio module.")
    set(SFML_BUILD_NETWORK TRUE CACHE BOOL "TRUE to build SFML's Network module.")

    # add compile flag -w : do not treat warnings as errors
    add_compile_options(-w)
    tool_include_lib(sfml)
    # remove compile flag -w
    tool_remove_compile_options(-w)

    target_compile_definitions(sfml-system PUBLIC -DSFML_STATIC)
    target_compile_definitions(sfml-window PUBLIC -DSFML_STATIC)
    target_compile_definitions(sfml-graphics PUBLIC -DSFML_STATIC)
    target_compile_definitions(sfml-audio PUBLIC -DSFML_STATIC)
    target_compile_definitions(sfml-network PUBLIC -DSFML_STATIC)

    # set the target's folder (for IDEs that support it, e.g. Visual Studio)
    set_target_properties(sfml-system PROPERTIES FOLDER "LIBS/SFML")
    set_target_properties(sfml-window PROPERTIES FOLDER "LIBS/SFML")
    set_target_properties(sfml-graphics PROPERTIES FOLDER "LIBS/SFML")
    set_target_properties(sfml-audio PROPERTIES FOLDER "LIBS/SFML")
    set_target_properties(sfml-network PROPERTIES FOLDER "LIBS/SFML")
    
    include_directories("${ARIBEIRO_LIBS_DIR}/${LIBNAME}/include/" PARENT_SCOPE)

    if(OS_TARGET STREQUAL win)
        copy_file_after_build(
            sfml-audio
            sfml/extlibs/bin/${ARCH_TARGET}/openal32.dll
        )
    endif()

elseif(LIB_SFML STREQUAL UsingFindPackage)

    message(FATAL_ERROR "SFML FIND PACKAGE NOT IMPLEMENTED")

    # tool_is_lib(assimp assimp_registered)
    # if (NOT ${assimp_registered})

    #     add_library(assimp OBJECT ${assimp_LIBRARIES})
    #     target_link_libraries(assimp ${assimp_LIBRARIES})
    #     include_directories(${assimp_INCLUDE_DIRS} PARENT_SCOPE)

    #     # set the target's folder (for IDEs that support it, e.g. Visual Studio)
    #     set_target_properties(assimp PROPERTIES FOLDER "LIBS")

    #     tool_register_lib(assimp)

    # endif()

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
