cmake_minimum_required (VERSION 2.8)

project (lib)

include_directories(.)
aux_source_directory(.  SOURCES)
message(STATUS "lib source:" ${SOURCES})
message(STATUS "lib name:" ${LIB_NAME})
add_library(${LIB_NAME} SHARED ${SOURCES})

install(TARGETS ${LIB_NAME} DESTINATION lib)

