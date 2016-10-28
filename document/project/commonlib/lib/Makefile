#
# Library directory and libs.
#
LIBS=-L.                      \
     -L /usr/lib              \
     -L /usr/local/lib        \
     -lpthread     	      \
     -lxml2           

#
# Include files directory
#     
INCLUDES=-I.                      \
         -I/usr/include           \
         -I/usr/local/include     \
         -I/usr/include/libxml2             

CC = g++
CCFLAGS = -Wall -shared -g -D_REENTRANT -Dlinux -fPIC  $(INCLUDES) 

#
# The build out objects files directory
#
OBJS-DIR=./do

#
# All the common objects files used in commonlib
#
OBJECTS    =  $(OBJS-DIR)/Condition.o   \
              $(OBJS-DIR)/Logger.o      \
              $(OBJS-DIR)/MemoryPool.o  \
              $(OBJS-DIR)/MutexLock.o   \
              $(OBJS-DIR)/RwLock.o      \
              $(OBJS-DIR)/Thread.o      \
              $(OBJS-DIR)/ThreadPool.o  \
	      $(OBJS-DIR)/XmlParser.o
	
LIB_TARGET = libcommonlib.so  

#
# Generate the target
#

all: $(LIB_TARGET)
$(LIB_TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $(LIB_TARGET) $(OBJECTS) $(LIBS)
$(OBJS-DIR)/%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@ 
install:
	sudo cp -f $(LIB_TARGET)  /usr/lib/
uninstall:
	sudo rm -f /usr/lib/libcommonlib.so
.PHONY : clean
clean :
	rm -f $(LIB_TARGET) $(OBJECTS)





