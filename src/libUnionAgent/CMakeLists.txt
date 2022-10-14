project(KCliBaseAgent)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_definitions("-DKOAL_MSGLIB_EXPORT")
add_definitions("-DKOAL_UNIONLIB_EXPORT")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../comm)
set(MSGLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../msglib)

set(MSGLIB_SRCS ${MSGLIB_DIR}/exportc.cpp)

include_directories(${COMM_DIR})
aux_source_directory(${COMM_DIR} COMM_SRCS)
aux_source_directory(${SRC_DIR} MAIN_SRCS)

add_library(KCliBaseAgent SHARED ${MAIN_SRCS} ${COMM_SRCS} ${MSGLIB_SRCS})

if(WIN32)
	target_link_libraries(KCliBaseAgent nanomsg)
elseif(UNIX)
	target_link_libraries(KCliBaseAgent nanomsg pthread)
endif(WIN32)

