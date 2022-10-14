project(KCliBaseTray)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

if(OPEN_TEST)
	add_definitions("-DTEST_MONITOR_MARCO")
endif(OPEN_TEST)

set(RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SIGLEAPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/sigleInstance)
set(COMM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../comm)
set(MSGLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../msglib)

set(MSGLIB_SRCS ${MSGLIB_DIR}/exportc.cpp)

include_directories(${SIGLEAPP_DIR})
include_directories(${COMM_DIR})
aux_source_directory(${SIGLEAPP_DIR} SIGLE_SRCS)
aux_source_directory(${SRC_DIR} MAIN_SRCS)
aux_source_directory(${COMM_DIR} COMM_SRCS)

find_package(Qt4 4.8.7 REQUIRED QtCore QtGui QtNetwork)

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

if(MSVC)
	add_executable(KCliBaseTray ${RESOURCE_DIR}/uniManagerTray.qrc ${SRC_DIR}/uniManagerTray.rc ${MAIN_SRCS} ${SIGLE_SRCS} ${COMM_SRCS} ${MSGLIB_SRCS})
else()
	add_executable(KCliBaseTray ${RESOURCE_DIR}/uniManagerTray.qrc ${MAIN_SRCS} ${SIGLE_SRCS} ${COMM_SRCS} ${MSGLIB_SRCS})
endif()

if(WIN32)
	target_link_libraries(KCliBaseTray nanomsg Qt4::QtCore Qt4::QtGui Qt4::QtNetwork)
elseif(UNIX)
	target_link_libraries(KCliBaseTray nanomsg pthread Qt4::QtCore Qt4::QtGui Qt4::QtNetwork)
endif(WIN32)

