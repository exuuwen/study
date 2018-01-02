protoc-c --c_out=.  students.proto
gcc students.pb-c.c students.c -o main `pkg-config --cflags 'libprotobuf-c >= 1.0.0'` `pkg-config --libs 'libprotobuf-c >= 1.0.0'`
