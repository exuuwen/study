# The name of our project is "HELLO".  CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
project (MAIN)

cmake_minimum_required(VERSION 2.8)

# Recurse into the "Hello" and "Demo" subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
add_subdirectory (Hello)
add_subdirectory (Demo)

# Make sure the compiler can find include files from our Hello library.
include_directories (${MAIN_SOURCE_DIR}/Hello)

# Make sure the linker can find the Hello Demo library once it is built.
#link_directories (${HELLO_BINARY_DIR}/Hello)
#link_directories (${HELLO_BINARY_DIR}/Demo)

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# Add executable called "MAIN" that is built from the source files
add_executable (Main ${DIR_SRCS})

# Link the executable to the Hello Demo library.
target_link_libraries (Main Hello Demo)

