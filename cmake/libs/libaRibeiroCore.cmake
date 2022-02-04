if (NOT LIB_ARIBEIROCORE)
    set( LIB_ARIBEIROCORE FromGit CACHE STRING "Choose the Library Source." FORCE )
    set_property(CACHE LIB_ARIBEIROCORE PROPERTY STRINGS None FromGit)
endif()

if (LIB_ARIBEIROCORE STREQUAL FromGit)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromPackage option for any lib.")
    endif()

    if (GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroCore.git" aRibeiroCore)
    elseif (GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroCore.git" aRibeiroCore)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${GIT_DOWNLOAD_METHOD}" )
    endif()

    set(supress_show_info ON)
    tool_include_lib(aRibeiroCore)
    unset(supress_show_info)

else()
    message(FATAL_ERROR "You need to select the source of the aRibeiroCore." )
endif()
