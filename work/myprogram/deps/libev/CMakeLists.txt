include(ExternalProject)

ExternalProject_Add(libev 
  PREFIX			.
  URL				${CMAKE_CURRENT_SOURCE_DIR}/libev-4.20.tar.gz
  BUILD_IN_SOURCE 	1
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR} --disable-shared
)
