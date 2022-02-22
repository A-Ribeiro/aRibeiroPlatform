if (TARGET aRibeiroPlatform)
    return()
endif()

set( LIB_ARIBEIROPLATFORM TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ARIBEIROPLATFORM PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromGit)

if(LIB_ARIBEIROPLATFORM STREQUAL TryFindPackageFirst)
    find_package(aRibeiroPlatform QUIET)
    if (ARIBEIROPLATFORM_FOUND)
        message(STATUS "[LIB_ARIBEIROPLATFORM] using system lib.")
        set(LIB_ARIBEIROPLATFORM UsingFindPackage)
    else()
        message(STATUS "[LIB_ARIBEIROPLATFORM] compiling from source.")
        set(LIB_ARIBEIROPLATFORM FromGit)
    endif()
endif()

if (LIB_ARIBEIROPLATFORM STREQUAL FromGit)

    unset(ARIBEIROPLATFORM_LIBRARIES CACHE)
    unset(ARIBEIROPLATFORM_INCLUDE_DIR CACHE)

    set( ARIBEIRO_GIT_DOWNLOAD_METHOD None CACHE STRING "The GitHUB download method." )
    set_property(CACHE ARIBEIRO_GIT_DOWNLOAD_METHOD PROPERTY STRINGS None SSH HTTPS)

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL None)
        message("\nYou need to set ARIBEIRO_GIT_DOWNLOAD_METHOD with:")
        message("    SSH   -> To use the SSH gitHUB URL.")
        message("    HTTPS -> To use the HTTPS gitHUB URL.")
        message( FATAL_ERROR "" )
    endif()

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroPlatform.git" aRibeiroPlatform)
    elseif (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroPlatform.git" aRibeiroPlatform)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${ARIBEIRO_GIT_DOWNLOAD_METHOD}" )
    endif()

    #set(supress_show_info ON CACHE INTERNAL "" FORCE)
    set(old_supress_show_info ${supress_show_info})
    set(supress_show_info ON)
    tool_include_lib(aRibeiroPlatform)
    set(supress_show_info ${old_supress_show_info})
    #unset(supress_show_info CACHE)

elseif (LIB_ARIBEIROPLATFORM STREQUAL UsingFindPackage)

    if (NOT TARGET aRibeiroPlatform)

        if(OS_TARGET STREQUAL linux)
            set(ADD_LIBS pthread rt)
        endif()

        find_package(aRibeiroPlatform REQUIRED QUIET)
        add_library(aRibeiroPlatform OBJECT ${ARIBEIROPLATFORM_LIBRARIES})
        target_link_libraries(aRibeiroPlatform ${ARIBEIROPLATFORM_LIBRARIES} ${ADD_LIBS})
        include_directories(${ARIBEIROPLATFORM_INCLUDE_DIR})
        set_target_properties(aRibeiroPlatform PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(aRibeiroPlatform PROPERTIES FOLDER "aRibeiro")

    endif()

else()

    message(FATAL_ERROR "You need to select the source of the aRibeiroPlatform." )
endif()
