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

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL HTTPS)
        tool_download_git_package("https://github.com/A-Ribeiro/aRibeiroData.git" aRibeiroData)
    elseif (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL SSH)
        tool_download_git_package("git@github.com:A-Ribeiro/aRibeiroData.git" aRibeiroData)
    else()
        message(FATAL_ERROR "Invalid Git Download Method: ${ARIBEIRO_GIT_DOWNLOAD_METHOD}" )
    endif()

    set(supress_show_info ON CACHE INTERNAL "" FORCE)
    tool_include_lib(aRibeiroData)
    unset(supress_show_info CACHE)

elseif (LIB_ARIBEIRODATA STREQUAL UsingFindPackage)

    tool_is_lib(aRibeiroData aRibeiroData_registered)
    if (NOT ${aRibeiroData_registered})

        find_package(aRibeiroData REQUIRED QUIET)
        add_library(aRibeiroData OBJECT ${ARIBEIRODATA_LIBRARIES})
        include_directories(${ARIBEIRODATA_INCLUDE_DIR} PARENT_SCOPE)

        tool_register_lib(aRibeiroData)
    endif()

else()

    message(FATAL_ERROR "You need to select the source of the aRibeiroData." )
endif()
