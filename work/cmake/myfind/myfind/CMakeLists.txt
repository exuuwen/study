cmake_minimum_required (VERSION 2.8)

project (myfind)

include_directories(.)
aux_source_directory(.  SOURCES)
message(STATUS "myfind source:" ${SOURCES})
message(STATUS "mifind name:" ${PROJECT_NAME})
add_library(${PROJECT_NAME} STATIC ${SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)

