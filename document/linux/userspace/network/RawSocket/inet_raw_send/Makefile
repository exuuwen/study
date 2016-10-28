#
# Library directory and libs.
#
LIBS=-L.                      \
     -L /usr/lib              \
     -L /usr/local/lib        \
     -lpthread                

#
# Include files directory
#     
INCLUDES=-I.                      \
         -I/usr/include           \
         -I/usr/local/include     \
         -I${LIB_DIR}/Lib/        \
	 -I${LIB_DIR}/Lib/CheckSum

CC = g++
CCFLAGS = -Wall -shared -g -D_REENTRANT -Dlinux -fPIC $(INCLUDES) 

OBJECTS = ${LIB_DIR}/Build/objects/RawSocket.o    

#
# Generate the target
#
all: $(OBJECTS)

$(OBJECTS):RawSocket.cpp
	$(CC) $(CCFLAGS) -c $< -o $@ $(LIBS) 

.PHONY : clean
clean :
	rm -f $(OBJECTS)





