CC := g++

TARGET_BIN := main
OBJECT := main.o
LIBS = -ldl
BIN_SRC := $(OBJECT:.o=.cpp)
BIN_FLAGS := -c

TARGET_LIB := libtest.so
CFLAGS := -fPIC -shared
LIB_SRC := libtest.cpp

all: $(TARGET_BIN) $(TARGET_LIB)
$(TARGET_BIN): $(OBJECT)
	$(CC) $(OBJECT) -o $@ $(LIBS)
%.o: %.c 
	$(CC) $(BIN_FLAGS) $< -o $@

$(TARGET_LIB):
	$(CC) $(CFLAGS) $(LIB_SRC) -o $(TARGET_LIB)
clean:
	rm -f *.o $(TARGET_BIN) $(TARGET_LIB)
