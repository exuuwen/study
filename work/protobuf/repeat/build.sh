protoc-c --c_out=.  repeat.proto
gcc repeat.pb-c.c pack_repeat.c -o pack_repeat `pkg-config --cflags 'libprotobuf-c >= 1.0.0'` `pkg-config --libs 'libprotobuf-c >= 1.0.0'`
gcc repeat.pb-c.c unpack_repeat.c -o unpack_repeat `pkg-config --cflags 'libprotobuf-c >= 1.0.0'` `pkg-config --libs 'libprotobuf-c >= 1.0.0'`
