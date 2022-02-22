if (TARGET aRibeiroData)
    return()
endif()

set( LIB_ARIBEIRODATA TryFindPackageFirst CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ARIBEIRODATA PROPERTY STRINGS None TryFindPackageFirst UsingFindPackage FromGit)

if(LIB_ARIBEIRODATA STREQUAL TryFindPackageFirst)
    find_package(aRibeiroData QUIET)
    if (ARIBEIRODATA_FOUND)
        message(STATUS "[LIB_ARIBEIRODATA] using system lib.")
        set(LIB_ARIBEIRODATA UsingFindPackage)
    else()
        message(STATUS "[LIB_ARIBEIRODATA] compiling from source.")
        set(LIB_ARIBEIRODATA FromGit)
    endif()
endif()

if (LIB_ARIBEIRODATA STREQUAL FromGit)

    unset(ARIBEIRODATA_LIBRARIES CACHE)
    unset(ARIBEIRODATA_INCLUDE_DIR CACHE)

    set( ARIBEIRO_GIT_DOWNLOAD_METHOD None CACHE STRING "The GitHUB download method." )
    set_property(CACHE ARIBEIRO_GIT_DOWNLOAD_METHOD PROPERTY STRINGS None SSH HTTPS)

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL None)
        message("\nYou need to set ARIBEIRO_GIT_DOWNLOAD_METHOD with:")
        message("    SSH   -> To use the SSH gitHUB URL.")
        message("    HTTPS -> To use the HTTPS gitHUB URL.")
        message( FATAL_ERROR "" )
    endif()

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroData.git" aRibeiroData)
    elseif (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroData.git" aRibeiroData)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${ARIBEIRO_GIT_DOWNLOAD_METHOD}" )
    endif()

    #set(supress_show_info ON CACHE INTERNAL "" FORCE)
    set(old_supress_show_info ${supress_show_info})
    set(supress_show_info ON)
    tool_include_lib(aRibeiroData)
    set(supress_show_info ${old_supress_show_info})
    #unset(supress_show_info CACHE)

elseif (LIB_ARIBEIRODATA STREQUAL UsingFindPackage)

    if (NOT TARGET aRibeiroData)

        find_package(aRibeiroData REQUIRED QUIET)
        add_library(aRibeiroData OBJECT ${ARIBEIRODATA_LIBRARIES})
        target_link_libraries(aRibeiroData ${ARIBEIRODATA_LIBRARIES})
        include_directories(${ARIBEIRODATA_INCLUDE_DIR})
        set_target_properties(aRibeiroData PROPERTIES LINKER_LANGUAGE CXX)

        # set the target's folder (for IDEs that support it, e.g. Visual Studio)
        set_target_properties(aRibeiroData PROPERTIES FOLDER "aRibeiro")

    endif()

else()

    message(FATAL_ERROR "You need to select the source of the aRibeiroData." )
endif()
