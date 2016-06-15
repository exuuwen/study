project (protos)

file(GLOB MY_PROTOS *.proto) 
message(STATUS "MY_PROTOS=${MY_PROTOS}")

foreach(PROTO_PATH IN LISTS MY_PROTOS) 
  get_filename_component(PROTO_NAME ${PROTO_PATH} NAME)
  string(REPLACE "proto" "pb.cc" PROTO_SOURCE ${PROTO_NAME})
  list(APPEND PROTO_SOURCES ${PROTO_SOURCE}) 
endforeach() 

message(STATUS "PROTO_SOURCES=${PROTO_SOURCES}")

add_custom_command( 
  OUTPUT ${PROTO_SOURCES}
  COMMAND ${CMAKE_BINARY_DIR}/bin/protoc --cpp_out=. -I ${CMAKE_CURRENT_SOURCE_DIR} ${MY_PROTOS} 
  #COMMAND mkdir -p ${CMAKE_BINARY_DIR}/include/
  COMMAND bash -c "mv *.h ${CMAKE_BINARY_DIR}/include/" 
  DEPENDS libprotobuf ${MY_PROTOS} 
)

add_library(${PROJECT_NAME} STATIC ${PROTO_SOURCES}) 
target_link_libraries(${PROJECT_NAME} protobuf) 
