# programtr
project(programtr)

FIND_PACKAGE(cutils REQUIRED)
FIND_PACKAGE(clibcdc REQUIRED)
FIND_PACKAGE(clibspi REQUIRED)

# Specify source and header files.
set(programtr_SRC_FILES
	${CMAKE_SOURCE_DIR}/examples/programtr/programtr.cpp
	${CMAKE_SOURCE_DIR}/examples/programtr/programtr_cmd.cpp
)

set(programtr_INC_FILES
	${CMAKE_SOURCE_DIR}/examples/programtr/programtr_cmd.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${cutils_INCLUDE_DIRS})
include_directories(${clibcdc_INCLUDE_DIRS})
include_directories(${clibspi_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/IqrfCdcChannel)
include_directories(${CMAKE_SOURCE_DIR}/IqrfSpiChannel)
include_directories(${CMAKE_SOURCE_DIR}/IqrfFakeChannel)

# Group the files in IDE.
source_group("include" FILES ${programtr_INC_FILES})

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src) #declaration of private impl.

add_executable(${PROJECT_NAME} ${programtr_SRC_FILES} ${programtr_INC_FILES})

if (WIN32) 
	target_link_libraries(${PROJECT_NAME} tr IqrfCdcChannel IqrfSpiChannel IqrfFakeChannel cdc spi_iqrf sysfs_gpio)
else()
	target_link_libraries(${PROJECT_NAME} tr IqrfCdcChannel IqrfSpiChannel IqrfFakeChannel cdc spi_iqrf sysfs_gpio pthread rt)
endif()
