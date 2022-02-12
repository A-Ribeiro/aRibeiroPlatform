if(NOT OpenGLStarter_Integration)

    set( ARIBEIRO_GIT_DOWNLOAD_METHOD None CACHE STRING "The GitHUB download method." )
    set_property(CACHE ARIBEIRO_GIT_DOWNLOAD_METHOD PROPERTY STRINGS None SSH HTTPS)

    if (ARIBEIRO_GIT_DOWNLOAD_METHOD STREQUAL None)
        message("\nYou need to set ARIBEIRO_GIT_DOWNLOAD_METHOD with:")
        message("    SSH   -> To use the SSH gitHUB URL.")
        message("    HTTPS -> To use the HTTPS gitHUB URL.")
        message( FATAL_ERROR "" )
    endif()

    include(cmake/00_DetectOSAndArchitecture.cmake)
    include(cmake/01_tools.cmake)
    include(cmake/02_copy_headers_to_include_directory.cmake)
    include(cmake/03_ide_setup.cmake)
    include(cmake/detect_neon.cmake)
    include(cmake/detect_openmp.cmake)
    include(cmake/detect_rpi.cmake)
    include(cmake/detect_sse2.cmake)
    include(cmake/aribeiro_options.cmake)

    set( OpenGLStarter_Integration ON )

endif()
