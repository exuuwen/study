CC := gcc

TARGET_BIN := hotpatch_function_dlopen
OBJECT := hotpatch_function_dlopen.o
TRACEE_BIN := traceer
TOBJECT := traceer.o
LIBS = -ldl
BIN_SRC := $(OBJECT:.o=.c)
BIN_FLAGS := -c

TARGET_LIB := libpatch.so
CFLAGS := -fPIC -shared
LIB_SRC := libpatch.c

all: $(TARGET_BIN) $(TARGET_LIB) $(TRACEE_BIN)
$(TARGET_BIN): $(OBJECT)
	$(CC) $(OBJECT) -o $@ $(LIBS)
%.o: %.c 
	$(CC) $(BIN_FLAGS) $< -o $@

$(TRACEE_BIN):$(TOBJECT)
	$(CC) $(TOBJECT) -o $@
%.o: %.c 
	$(CC) $(BIN_FLAGS) $< -o $@

$(TARGET_LIB):
	$(CC) $(CFLAGS) $(LIB_SRC) -o $(TARGET_LIB)
clean:
	rm -f *.o $(TARGET_BIN) $(TARGET_LIB)
