project(KCliBaseService)

# Turn on automatic invocation of the MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find includes in the build directories
if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(COMM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../comm)
set(MSGLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../msglib)

set(MSGLIB_SRCS ${MSGLIB_DIR}/deviceskt.cpp
				${MSGLIB_DIR}/exports.cpp
				${MSGLIB_DIR}/msglibs.cpp
	)

include_directories(${COMM_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src MAIN_SRCS)
aux_source_directory(${COMM_DIR} COMM_SRCS)

find_package(Qt4 4.8.7 REQUIRED QtCore QtNetwork)

add_executable(KCliBaseService ${MAIN_SRCS} ${COMM_SRCS} ${MSGLIB_SRCS})

if(WIN32)
	target_link_libraries(KCliBaseService QtSolutions_Service-head sqlite3 nanomsg Qt4::QtCore Qt4::QtNetwork)
elseif(UNIX)
	target_link_libraries(KCliBaseService QtSolutions_Service-head sqlite3 nanomsg pthread Qt4::QtCore Qt4::QtNetwork)
endif(WIN32)

