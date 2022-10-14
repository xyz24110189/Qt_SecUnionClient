project(Test)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(UTILS_DIR ${CMAKE_SOURCE_DIR}/src/libUnionAgent/src)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/bin/Release)

include_directories(${UTILS_DIR})
link_directories(${LIB_DIR})
aux_source_directory(${SRC_DIR} MAIN_SRCS)

add_executable(KCliBaseTest ${MAIN_SRCS})

target_link_libraries(KCliBaseTest KCliBaseAgent)
