
CC = g++
CCFLAGS = -Wall -shared -g -D_REENTRANT -Dlinux -fPIC

#
# All the common objects files used in commonlib
#
OBJECTS    =  Aclass.o   \
              AclassImpl.o      \
              Bclass.o   
	
LIB_TARGET = libmy.so  

#
# Generate the target
#

lib: $(LIB_TARGET) 
$(LIB_TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $(LIB_TARGET) $(OBJECTS)
%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@ 

client: client.o
	$(CC) -o client  client.o -lmy

install:
	sudo cp -f $(LIB_TARGET)  /usr/lib/
uninstall:
	sudo rm -f /usr/lib/$(LIB_TARGET)
.PHONY : clean
clean :
	rm -f $(LIB_TARGET) $(OBJECTS)
