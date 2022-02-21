if (TARGET convertutf)
    return()
endif()

set( LIB_CONVERTUTF FromSource CACHE STRING "Choose the Library Source." )
set_property(CACHE LIB_CONVERTUTF PROPERTY STRINGS None FromSource)

message(STATUS "[LIB_CONVERTUTF] compiling from source.")

if(LIB_CONVERTUTF STREQUAL FromSource)

    if (NOT LIBS_REPOSITORY_URL)
        message(FATAL_ERROR "You need to define the LIBS_REPOSITORY_URL to use the FromSource option for any lib.")
    endif()

    tool_download_lib_package(${LIBS_REPOSITORY_URL} convertutf)

    tool_include_lib(convertutf)

    #include_directories(${CMAKE_HOME_DIRECTORY}/include/convertutf/ PARENT_SCOPE)

else()
    message( FATAL_ERROR "You need to specify the lib source." )
endif()
