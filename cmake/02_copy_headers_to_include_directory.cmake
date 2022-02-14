############################################################################
# Copy headers to include directory
############################################################################
macro(copy_headers_to_include_directory projectname)
    file(COPY ${ARGN} DESTINATION "${ARIBEIRO_GEN_INCLUDE_DIR}/${projectname}/" )
endmacro()

include_directories("${ARIBEIRO_GEN_INCLUDE_DIR}")
