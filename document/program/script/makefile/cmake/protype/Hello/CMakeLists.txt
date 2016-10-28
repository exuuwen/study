
AUX_SOURCE_DIRECTORY(. DIR_HELLO_SRCS)

# Add library called "Demo" that is built from the source files
add_library (Hello ${DIR_HELLO_SRCS})
