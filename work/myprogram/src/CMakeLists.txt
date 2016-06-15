cmake_minimum_required (VERSION 2.8)

project (src)
message(STATUS "This is BINARY dir " ${src_BINARY_DIR})
message(STATUS "This is SOURCE dir " ${src_SOURCE_DIR})

set(CMAKE_C_FLAGS        "${CMAKE_C_FLAGS}   -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wpointer-arith -Wnested-externs -Wcast-qual -Wformat-security -Wundef -Wwrite-strings -Wno-error=unused-but-set-variable") 
set(CMAKE_CXX_FLAGS      "${CMAKE_CXX_FLAGS} -W -Wall -Wmissing-declarations -Wpointer-arith -Wcast-qual -Wformat-security -Wundef -Wwrite-strings -Wno-error=unused-but-set-variable") 

include_directories(.)

link_directories(${CMAKE_BINARY_DIR}/lib)

aux_source_directory(. SOURCES)
message(STATUS "project source:" ${SOURCES})
message(STATUS "programe name:" ${PROGRAM_NAME})


list(REMOVE_ITEM SOURCES main.cpp) 
add_library(${PROGRAM_NAME}_impl STATIC ${SOURCES}) 
target_link_libraries(${PROGRAM_NAME}_impl ev m protos)

add_executable(${PROGRAM_NAME} main.cpp)
target_link_libraries(${PROGRAM_NAME} ${LIB_NAME}) 
target_link_libraries(${PROGRAM_NAME} ${PROGRAM_NAME}_impl) 

install(TARGETS ${PROGRAM_NAME} DESTINATION bin)
