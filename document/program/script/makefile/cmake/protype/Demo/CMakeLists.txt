# Make sure the compiler can find include files from our Hello library.
include_directories (${MAIN_SOURCE_DIR}/Hello)

AUX_SOURCE_DIRECTORY(. DIR_DEMO_SRCS)

# Add library called "Demo" that is built from the source files
add_library  (Demo ${DIR_DEMO_SRCS})

