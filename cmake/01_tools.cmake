
set( ARIBEIRO_LIBS_DIR None CACHE STRING "The directory to download the libraries configured." )
set( ARIBEIRO_GEN_INCLUDE_DIR None CACHE STRING "The directory to be created with a copy of the public headers of the library." )

if (ARIBEIRO_LIBS_DIR STREQUAL None)
    message(FATAL_ERROR "You need to set the ARIBEIRO_LIBS_DIR to any directory. Example: ${CMAKE_HOME_DIRECTORY}/libs")
endif()

if (ARIBEIRO_GEN_INCLUDE_DIR STREQUAL None)
    message(FATAL_ERROR "You need to set the ARIBEIRO_GEN_INCLUDE_DIR to any directory. Example: ${CMAKE_HOME_DIRECTORY}/include")
endif()


macro ( mark_as_internal _var )
  set ( ${_var} ${${_var}} CACHE INTERNAL "hide this!" FORCE )
endmacro( mark_as_internal _var )

macro(tool_get_directory_definitions var)
#get_directory_property(compile_defs COMPILE_DEFINITIONS)
#get_property(compile_defs DIRECTORY PROPERTY COMPILE_DEFINITIONS)
    get_directory_property(aux COMPILE_DEFINITIONS)
    set(${var} "")
    foreach(entry ${aux})
        list(APPEND ${var} "-D${entry}" )
    endforeach()
endmacro()

macro(list_to_string list str)
    set(${str} "")
    foreach(entry ${list})
        string(LENGTH "${${str}}" length)
        if( ${length} EQUAL 0 )
            string(APPEND ${str} "${entry}" )
        else()
            string(APPEND ${str} " ${entry}" )
        endif()
    endforeach()
endmacro()

macro(create_missing_cmake_build_type)
    #if( NOT DEFINED CMAKE_BUILD_TYPE ) #the variable need to be check with empty content
    if( NOT CMAKE_BUILD_TYPE )
        set( CMAKE_BUILD_TYPE Release CACHE STRING
                "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
                FORCE )
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
    endif()
endmacro()

macro(configure_build_flags projectname inputfile outputfile)
    set(cmake_build_flags "")

    set(target_compile_defs)
    if (TARGET ${projectname})
        get_target_property(target_compile_defs ${projectname} COMPILE_DEFINITIONS)
    endif()

    # get build flags
    get_directory_property(aux COMPILE_DEFINITIONS)

    # copy target defs that are not in dir defs
    foreach(target_compile_opt ${target_compile_defs})
        if (NOT "${target_compile_opt}" IN_LIST aux)
            list(APPEND aux ${target_compile_opt})
        endif()
    endforeach()


    foreach(define ${aux})
        if(NOT "${define}" STREQUAL "NDEBUG")
            set(cmake_build_flags "${cmake_build_flags}#ifndef ${define}\n    #define ${define}\n#endif\n")
        endif()
    endforeach()

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${inputfile}"
        "${ARIBEIRO_GEN_INCLUDE_DIR}/${projectname}/${outputfile}"
        @ONLY
    )
endmacro()

macro(configure_include_file projectname inputfile outputfile)
    set(cmake_includes "")

    #get all files
    foreach(filename IN ITEMS ${ARGN})
        get_filename_component(FILENAME_WITHOUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${filename}" NAME)
        set(cmake_includes "${cmake_includes}#include <${projectname}/${FILENAME_WITHOUT_PATH}>\n")
    endforeach()


    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${inputfile}"
        "${ARIBEIRO_GEN_INCLUDE_DIR}/${projectname}/${outputfile}"
        @ONLY
    )
endmacro()

macro(define_source_group )
    foreach(entry IN ITEMS ${ARGN})
        get_filename_component(dirname "${entry}" DIRECTORY )
        if (dirname)
            string(REPLACE "/" "\\" dirname_replaced ${dirname})
            source_group(${dirname_replaced} FILES ${entry})
        else()
            source_group("" FILES ${entry})
        endif()
    endforeach()
endmacro()

macro(define_source_group_base_path base_path )
    foreach(entry IN ITEMS ${ARGN})
        get_filename_component(dirname "${entry}" DIRECTORY )
        if (dirname)
            
            string(FIND "${dirname}" "${base_path}" found)

            if (found VERSION_EQUAL 0)
                string(LENGTH "${base_path}" base_path_length)
                string(LENGTH "${dirname}" dirname_length)
                math(EXPR new_length "${dirname_length} - ${base_path_length}")
                string(SUBSTRING "${dirname}" 
                        "${base_path_length}" 
                        "${new_length}" dirname)
            endif()

            string(FIND "${dirname}" "/" found)
            if (found VERSION_EQUAL 0)
                string(LENGTH "${dirname}" dirname_length)
                math(EXPR new_length "${dirname_length} - 1")
                string(SUBSTRING "${dirname}" 
                        "1" 
                        "${new_length}" dirname)
            endif()

            string(LENGTH "${dirname}" dirname_length)
            if (dirname_length VERSION_GREATER 0)
                string(REPLACE "/" "\\" dirname_replaced ${dirname})
                source_group(${dirname_replaced} FILES ${entry})
            else()
                source_group("" FILES ${entry})
            endif()

        else()
            source_group("" FILES ${entry})
        endif()
    endforeach()
endmacro()

macro(copy_file_after_build PROJECT_NAME)
    foreach(FILENAME IN ITEMS ${ARGN})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
			get_filename_component(FILENAME_WITHOUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}" NAME)
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${FILENAME_WITHOUT_PATH} )
        elseif(EXISTS "${FILENAME}")
			get_filename_component(FILENAME_WITHOUT_PATH "${FILENAME}" NAME)
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${FILENAME}
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${FILENAME_WITHOUT_PATH} )
        else()
            message(FATAL_ERROR "File Does Not Exists: ${FILENAME}")
        endif()
    endforeach()
endmacro()

macro(copy_directory_after_build PROJECT_NAME)
    foreach(DIRECTORY IN ITEMS ${ARGN})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}")
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DIRECTORY} )
        else()
            message(FATAL_ERROR "Directory Does Not Exists: ${DIRECTORY}")
        endif()
    endforeach()
endmacro()

macro(copy_alessandro_ribeiro_content_after_build PROJECT_NAME DIRECTORY)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
				${CMAKE_HOME_DIRECTORY}/AlessandroRibeiro
				 $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DIRECTORY}/AlessandroRibeiro )
endmacro()

# if (OS_TARGET STREQUAL win)
#     macro(tool_unzip ZIPFILE OUTDIR)
#         execute_process(
#             COMMAND powershell.exe -file "${CMAKE_HOME_DIRECTORY}/cmake/powershell/unzip.ps1" -inputzipfile "${ZIPFILE}" -outputpath "${OUTDIR}"
#             OUTPUT_VARIABLE result
#         )
#     endmacro()
# else()
#     macro(tool_unzip ZIPFILE OUTDIR)
#         execute_process(
#             COMMAND unzip -n "${ZIPFILE}" -d "${OUTDIR}"
#             OUTPUT_VARIABLE result
#             COMMAND_ERROR_IS_FATAL ANY
#         )
#     endmacro()
# endif()

macro(tool_unzip ZIPFILE OUTDIR)
    #execute_process(
    #    COMMAND unzip -n "${ZIPFILE}" -d "${OUTDIR}"
    #    OUTPUT_VARIABLE result
    #    COMMAND_ERROR_IS_FATAL ANY
    #)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf "${ZIPFILE}"
        WORKING_DIRECTORY "${OUTDIR}"
        RESULT_VARIABLE ret
    )
    if(NOT ret EQUAL 0)
        file(REMOVE "${ZIPFILE}")
        message( FATAL_ERROR "Cannot unzip ${ZIPFILE}")
    endif()

endmacro()

macro(tool_download_lib_package REPOSITORY_URL LIBNAME)
    if(NOT EXISTS "${ARIBEIRO_LIBS_DIR}/${LIBNAME}.zip")
        file(MAKE_DIRECTORY "${ARIBEIRO_LIBS_DIR}")
        message(STATUS "[${LIBNAME}] downloading...")
        file(DOWNLOAD "${REPOSITORY_URL}/${LIBNAME}.zip" "${ARIBEIRO_LIBS_DIR}/${LIBNAME}.zip" SHOW_PROGRESS STATUS result)
        list (GET result 0 error_code)
        if(NOT error_code EQUAL 0)
            file(REMOVE "${ARIBEIRO_LIBS_DIR}/${LIBNAME}.zip")
            message(FATAL_ERROR "Cannot download: ${REPOSITORY_URL}/${LIBNAME}.zip")
        endif()
        message(STATUS "[${LIBNAME}] unzip...")
        tool_unzip(
            "${ARIBEIRO_LIBS_DIR}/${LIBNAME}.zip"
            "${ARIBEIRO_LIBS_DIR}"
        )
        message(STATUS "[${LIBNAME}] done")
    endif()
endmacro()


# set( lib_list "" CACHE INTERNAL "lib_lists")
# mark_as_internal(lib_list)

# macro(tool_is_lib LIBNAME result)
#     if ("${LIBNAME}" IN_LIST lib_list)
#         set(${result} ON)
#     else()
#         set(${result} OFF)
#     endif()
# endmacro()

# macro(tool_register_lib LIBNAME)
#     if (NOT "${LIBNAME}" IN_LIST lib_list)    
#         list(APPEND lib_list ${LIBNAME})
#         set( lib_list ${lib_list} CACHE INTERNAL "lib_lists" FORCE)
#     endif()
# endmacro()

macro(tool_include_lib)
    #add_subdirectory("${ARIBEIRO_LIBS_DIR}/${LIBNAME}" "${CMAKE_BINARY_DIR}/bin/${LIBNAME}")
    #add_subdirectory("${ARIBEIRO_LIBS_DIR}/${LIBNAME}" "${CMAKE_CURRENT_BINARY_DIR}/${LIBNAME}")

    #get_property(aux GLOBAL PROPERTY BUILDSYSTEM_TARGETS)
    #get_directory_property(aux BUILDSYSTEM_TARGETS)

    if (  ${ARGC} EQUAL 1 )
        set(LIBNAME ${ARGV0})
        if (NOT TARGET ${LIBNAME})
        # if (NOT "${LIBNAME}" IN_LIST lib_list)
            
        #     list(APPEND lib_list ${LIBNAME})
        #     set( lib_list ${lib_list} CACHE INTERNAL "lib_lists" FORCE)

            #message("Add new Lib: ${lib_list}")
            #message("Normal")
            #add_subdirectory("${ARIBEIRO_LIBS_DIR}/${LIBNAME}" "${CMAKE_BINARY_DIR}/${LIBNAME}")
            add_subdirectory("${ARIBEIRO_LIBS_DIR}/${LIBNAME}" "${ARIBEIRO_LIBS_DIR}/build/${LIBNAME}")
        endif()
    elseif (  ${ARGC} EQUAL 2 )
        set(_PATH ${ARGV0})
        set(LIBNAME ${ARGV1})
        if (NOT TARGET ${LIBNAME})
        # if (NOT "${LIBNAME}" IN_LIST lib_list)
            
        #     list(APPEND lib_list ${LIBNAME})
        #     set( lib_list ${lib_list} CACHE INTERNAL "lib_lists" FORCE)

            #message("Add new Lib: ${lib_list}")
            #message("Path")
            #add_subdirectory("${ARIBEIRO_LIBS_DIR}/${_PATH}/${LIBNAME}" "${CMAKE_BINARY_DIR}/${LIBNAME}")
            add_subdirectory("${ARIBEIRO_LIBS_DIR}/${_PATH}/${LIBNAME}" "${ARIBEIRO_LIBS_DIR}/build/${LIBNAME}")
        endif()
    else()
        message(FATAL_ERROR "incorrect number of arguments.")
    endif()
endmacro()


macro(tool_download_git_package REPOSITORY_URL LIBNAME)
    if(NOT EXISTS "${ARIBEIRO_LIBS_DIR}/${LIBNAME}/")
        file(MAKE_DIRECTORY "${ARIBEIRO_LIBS_DIR}")
        message(STATUS "[${LIBNAME}] cloning...")

        find_package(Git REQUIRED)

        execute_process(
            COMMAND "${GIT_EXECUTABLE}" clone "${REPOSITORY_URL}" "${ARIBEIRO_LIBS_DIR}/${LIBNAME}/"
            OUTPUT_VARIABLE result
            #COMMAND_ERROR_IS_FATAL ANY
        )

        if(NOT EXISTS "${ARIBEIRO_LIBS_DIR}/${LIBNAME}/")
            message(FATAL_ERROR "Error to clone repository: ${REPOSITORY_URL}")
        endif()

        message(STATUS "[${LIBNAME}] done")

    endif()
endmacro()


macro(tool_remove_compile_options)
    get_directory_property(compile_opts COMPILE_OPTIONS)

    foreach(entry IN ITEMS ${ARGN})
        list(REMOVE_ITEM compile_opts ${entry})
    endforeach()

    #set_property(DIRECTORY ${CMAKE_CURRENT_SRC_DIR} APPEND PROPERTY COMPILE_OPTIONS ${compile_opts})
    set_property(DIRECTORY ${CMAKE_CURRENT_SRC_DIR} PROPERTY COMPILE_OPTIONS ${compile_opts})
endmacro()


macro(tool_show_current_directories)
    get_directory_property( dirs INCLUDE_DIRECTORIES )
    message(STATUS "INCLUDE_DIRECTORIES for '${PROJECT_NAME}'")
    foreach(dir ${dirs})
        message(STATUS "    '${dir}'")
    endforeach()
endmacro()