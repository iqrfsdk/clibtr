#static tr library
project(tr)

FIND_PACKAGE(cutils REQUIRED)

# Specify source and header files.
set(tr_SRC_FILES
	${CMAKE_SOURCE_DIR}/src/TrException.cpp
	${CMAKE_SOURCE_DIR}/src/TrFmtException.cpp
	${CMAKE_SOURCE_DIR}/src/string_operations.cpp
	${CMAKE_SOURCE_DIR}/src/IqrfFmtParser.cpp
	${CMAKE_SOURCE_DIR}/src/HexFmtParser.cpp
	${CMAKE_SOURCE_DIR}/src/TrconfFmtParser.cpp
	${CMAKE_SOURCE_DIR}/src/TrIfc.cpp
)

set(tr_INC_FILES
	${CMAKE_SOURCE_DIR}/include/TrException.h
	${CMAKE_SOURCE_DIR}/include/TrFmtException.h
	${CMAKE_SOURCE_DIR}/src/string_operations.h
	${CMAKE_SOURCE_DIR}/include/IqrfFmtParser.h
	${CMAKE_SOURCE_DIR}/include/HexFmtParser.h
	${CMAKE_SOURCE_DIR}/include/TrconfFmtParser.h
	${CMAKE_SOURCE_DIR}/include/TrTypes.h
	${CMAKE_SOURCE_DIR}/include/TrIfc.h
)

# Group the files in IDE.
source_group("include" FILES ${tr_INC_FILES})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src) #declaration of private impl.
include_directories(${cutils_INCLUDE_DIRS})
include_directories(${clibcdc_INCLUDE_DIRS})

add_library(
	${PROJECT_NAME}
    STATIC
	${tr_SRC_FILES} ${tr_INC_FILES}
)

#TODO
#install(TARGETS ${project} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
