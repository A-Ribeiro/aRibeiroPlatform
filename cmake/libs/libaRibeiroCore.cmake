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

    tool_is_lib(aRibeiroCore aRibeiroPlatform_registered)
    if (NOT ${aRibeiroCore_registered})

        find_package(aRibeiroCore REQUIRED QUIET)
        add_library(aRibeiroCore OBJECT ${ARIBEIROCORE_LIBRARIES})
        include_directories(${ARIBEIROCORE_INCLUDE_DIR} PARENT_SCOPE)

        tool_register_lib(aRibeiroCore)
    endif()
else()

    message(FATAL_ERROR "You need to select the source of the aRibeiroCore." )
endif()
