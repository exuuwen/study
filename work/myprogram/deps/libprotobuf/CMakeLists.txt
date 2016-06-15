include(ExternalProject)

ExternalProject_Add(libprotobuf
  PREFIX			.
  URL				${CMAKE_CURRENT_SOURCE_DIR}/protobuf-2.4.1.tar.gz
  BUILD_IN_SOURCE 	1
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR} --disable-shared
)
