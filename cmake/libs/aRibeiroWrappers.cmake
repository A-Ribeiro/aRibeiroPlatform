set( LIB_ARIBEIROWRAPPERS FromGit CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_ARIBEIROWRAPPERS PROPERTY STRINGS None FromGit)

if (LIB_ARIBEIROWRAPPERS STREQUAL FromGit)

    set( ARIBEIRO_GIT_DOWNLOAD_METHOD None CACHE STRING "The GitHUB download method." )
    set_property(CACHE ARIBEIRO_GIT_DOWNLOAD_METHOD PROPERTY STRINGS None SSH HTTPS)

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL None)
        message("\nYou need to set ARIBEIRO_GIT_DOWNLOAD_METHOD with:")
        message("    SSH   -> To use the SSH gitHUB URL.")
        message("    HTTPS -> To use the HTTPS gitHUB URL.")
        message( FATAL_ERROR "" )
    endif()

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroWrappers.git" aRibeiroWrappers)
    elseif (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroWrappers.git" aRibeiroWrappers)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${ARIBEIRO_GIT_DOWNLOAD_METHOD}" )
    endif()

    #set(supress_show_info ON)
    #tool_include_lib(aRibeiroCore)
    #unset(supress_show_info)

else()
    message(FATAL_ERROR "You need to select the source of the aRibeiroCore." )
endif()
