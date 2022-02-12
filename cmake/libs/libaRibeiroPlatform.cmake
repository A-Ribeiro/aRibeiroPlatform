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

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroPlatform.git" aRibeiroPlatform)
    elseif (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroPlatform.git" aRibeiroPlatform)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${ARIBEIRO_GIT_DOWNLOAD_METHOD}" )
    endif()

    set(supress_show_info ON CACHE INTERNAL "" FORCE)
    tool_include_lib(aRibeiroPlatform)
    unset(supress_show_info CACHE)

elseif (LIB_ARIBEIROPLATFORM STREQUAL UsingFindPackage)

    tool_is_lib(aRibeiroPlatform aRibeiroPlatform_registered)
    if (NOT ${aRibeiroPlatform_registered})

        find_package(aRibeiroPlatform REQUIRED QUIET)
        add_library(aRibeiroPlatform OBJECT ${ARIBEIROPLATFORM_LIBRARIES})
        include_directories(${ARIBEIROPLATFORM_INCLUDE_DIR} PARENT_SCOPE)

        tool_register_lib(aRibeiroPlatform)
    endif()

else()

    message(FATAL_ERROR "You need to select the source of the aRibeiroPlatform." )
endif()
