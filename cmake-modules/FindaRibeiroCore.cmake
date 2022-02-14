# "For this is how God loved the world:
# he gave his only Son, so that everyone
# who believes in him may not perish
# but may have eternal life."
#
# John 3:16

if(ARIBEIROCORE_INCLUDE_DIR AND ARIBEIROCORE_LIBRARIES)
	unset(ARIBEIROCORE_INCLUDE_DIR)
	unset(ARIBEIROCORE_LIBRARIES)
endif()

find_path(ARIBEIROCORE_INCLUDE_DIR aRibeiroCore/aRibeiroCore.h)
find_library(ARIBEIROCORE_LIBRARIES NAMES aRibeiroCore)

if(ARIBEIROCORE_INCLUDE_DIR AND ARIBEIROCORE_LIBRARIES)
	set(ARIBEIROCORE_FOUND ON)
endif()

if(ARIBEIROCORE_FOUND)
	if (NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
		MESSAGE(STATUS "Found ${CMAKE_FIND_PACKAGE_NAME} include:  ${ARIBEIROCORE_INCLUDE_DIR}/aRibeiroCore/aRibeiroCore.h")
		MESSAGE(STATUS "Found ${CMAKE_FIND_PACKAGE_NAME} library: ${ARIBEIROCORE_LIBRARIES}")
	endif()
else()
	if(${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find ${CMAKE_FIND_PACKAGE_NAME} development files")
	endif()
endif()
