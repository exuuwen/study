cmake_minimum_required(VERSION 2.8)

project(example)

set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(PROJECT_VERSION "1.0")

add_subdirectory(deps)

set(CMAKE_C_FLAGS        "${CMAKE_C_FLAGS_${BUILD_TYPE}}       -m64 -march=native -pthread -std=gnu99") 
set(CMAKE_CXX_FLAGS      "${CMAKE_CXX_FLAGS_${BUILD_TYPE}}     -m64 -march=native -pthread -std=gnu++11") 
set(CMAKE_C_LINK_FLAGS   "${CMAKE_C_LINK_FLAGS}                -m64 -march=native -Wl,-melf_x86_64 -pthread ") 
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}              -m64 -march=native -Wl,-melf_x86_64 -pthread ")

set(LIB_NAME "test")
set(PROGRAM_NAME "${PROJECT_NAME}")

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(lib)
add_subdirectory(protos)
add_subdirectory(src)

message(STATUS "myprogram This is BINARY dir " ${CMAKE_BINARY_DIR})
message(STATUS "myprogram This is SOURCE dir " ${CMAKE_SOURCE_DIR})

set(CMAKE_INSTALL_PREFIX "/tmp/cmakeinstall")

install(DIRECTORY etc DESTINATION .)
install(FILES COPYRIGHT README DESTINATION "share/doc/myprogram") 
install(DIRECTORY bin/ DESTINATION bin USE_SOURCE_PERMISSIONS)
install(DIRECTORY doc/ DESTINATION "share/doc/myprogram")

set(CPACK_PACKAGE_NAME                      "${PROJECT_NAME}") 
set(CPACK_PACKAGE_VERSION                   "${PROJECT_VERSION}") 
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY       "example") 
set(CPACK_PACKAGE_CONTACT                   "wenxu@ucloud.cn")
set(CPACK_PACKAGING_INSTALL_PREFIX          "${CMAKE_INSTALL_PREFIX}")
set(CPACK_GENERATOR                         "RPM") 
set(CPACK_SYSTEM_NAME                       "x86_64") 
set(CPACK_RPM_PACKAGE_ARCHITECTURE          "x86_64") 
#set(CPACK_RPM_PACKAGE_REQUIRES              "pciutils protobuf-python") 
#set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE      "${CMAKE_CURRENT_SOURCE_DIR}/cpack/postinst") 
#set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE     "${CMAKE_CURRENT_SOURCE_DIR}/cpack/prerm") 
#set(CPACK_RPM_USER_FILELIST                 "%config ${CMAKE_INSTALL_PREFIX}/etc/")
include(CPack)

