if (TARGET aRibeiroCore)
    return()
endif()

set( LIB_ARIBEIROCORE TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ARIBEIROCORE PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromGit)

if(LIB_ARIBEIROCORE STREQUAL TryFindPackageFirst)
    find_package(aRibeiroCore QUIET)
    if (ARIBEIROCORE_FOUND)
        message(STATUS "[LIB_ARIBEIROCORE] using system lib.")
        set(LIB_ARIBEIROCORE UsingFindPackage)
    else()
        message(STATUS "[LIB_ARIBEIROCORE] compiling from source.")
        set(LIB_ARIBEIROCORE FromGit)
    endif()
endif()

if (LIB_ARIBEIROCORE STREQUAL FromGit)

    unset(ARIBEIROCORE_LIBRARIES CACHE)
    unset(ARIBEIROCORE_INCLUDE_DIR CACHE)

    set( ARIBEIRO_GIT_DOWNLOAD_METHOD None CACHE STRING "The GitHUB download method." )
    set_property(CACHE ARIBEIRO_GIT_DOWNLOAD_METHOD PROPERTY STRINGS None SSH HTTPS)

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL None)
        message("\nYou need to set ARIBEIRO_GIT_DOWNLOAD_METHOD with:")
        message("    SSH   -> To use the SSH gitHUB URL.")
        message("    HTTPS -> To use the HTTPS gitHUB URL.")
        message( FATAL_ERROR "" )
    endif()

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroCore.git" aRibeiroCore)
    elseif (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroCore.git" aRibeiroCore)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${ARIBEIRO_GIT_DOWNLOAD_METHOD}" )
    endif()

    #set(supress_show_info ON CACHE INTERNAL "" FORCE)
    
    set(old_supress_show_info ${supress_show_info})
    set(supress_show_info ON)
    tool_include_lib(aRibeiroCore)
    set(supress_show_info ${old_supress_show_info})
    #unset(supress_show_info CACHE)

elseif (LIB_ARIBEIROCORE STREQUAL UsingFindPackage)

    if (NOT TARGET aRibeiroCore)

        find_package(aRibeiroCore REQUIRED QUIET)
        add_library(aRibeiroCore OBJECT ${ARIBEIROCORE_LIBRARIES})
        target_link_libraries(aRibeiroCore ${ARIBEIROCORE_LIBRARIES})
        include_directories(${ARIBEIROCORE_INCLUDE_DIR})
        set_target_properties(aRibeiroCore PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(aRibeiroCore PROPERTIES FOLDER "aRibeiro")

    endif()
else()

    message(FATAL_ERROR "You need to select the source of the aRibeiroCore." )
endif()
