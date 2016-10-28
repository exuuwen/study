#
# Library directory and libs.
#
LIBS=-L /usr/lib              \
     -L /usr/local/lib        \
     -lpthread		      \
     -lxml2		      \
     -lcommonlib              

#
# Include files directory
#     
INCLUDES=-I.                      \
         -I/usr/include           \
         -I/usr/local/include     \
	 -I/usr/include/libxml2   \
	 -I../lib/        

CC = g++
CCFLAGS = -Wall -g -D_REENTRANT -Dlinux -fPIC  $(INCLUDES) 

VPATH=.
#
# The build out objects files directory
#
BINS-DIR=./do

#
# All the common objects files used in commonlib
#
BINS    =  $(BINS-DIR)/Condition_test	\
           $(BINS-DIR)/Atomic_test      \
           $(BINS-DIR)/ConcurrentQueue_test      \
           $(BINS-DIR)/LoggerFd_test      \
           $(BINS-DIR)/MemoryPool_test	\
           $(BINS-DIR)/MutexLock_test      \
           $(BINS-DIR)/PriorityQueue_test      \
           $(BINS-DIR)/RingQueue_test      \
           $(BINS-DIR)/RwLock_test	\
           $(BINS-DIR)/SingleQueue_test      \
           $(BINS-DIR)/Singleton_test      \
           $(BINS-DIR)/ThreadLocal_test      \
           $(BINS-DIR)/ThreadPool_test	\
           $(BINS-DIR)/Thread_basetest      \
	   $(BINS-DIR)/Thread_advancetest      \
   	   $(BINS-DIR)/XmlParser_test


#
# Generate the target
#

all: $(BINS)
$(BINS):
	$(CC) $(CCFLAGS) -o $@ $(filter %.cpp, $^) $(LIBS)
$(BINS-DIR)/Condition_test : Condition_test.cpp
$(BINS-DIR)/Atomic_test : Atomic_test.cpp
$(BINS-DIR)/ConcurrentQueue_test : ConcurrentQueue_test.cpp
$(BINS-DIR)/LoggerFd_test : LoggerFd_test.cpp
$(BINS-DIR)/MemoryPool_test : MemoryPool_test.cpp
$(BINS-DIR)/MutexLock_test : MutexLock_test.cpp
$(BINS-DIR)/PriorityQueue_test : PriorityQueue_test.cpp
$(BINS-DIR)/RingQueue_test : RingQueue_test.cpp
$(BINS-DIR)/RwLock_test : RwLock_test.cpp
$(BINS-DIR)/SingleQueue_test : SingleQueue_test.cpp
$(BINS-DIR)/Singleton_test : Singleton_test.cpp
$(BINS-DIR)/ThreadLocal_test : ThreadLocal_test.cpp
$(BINS-DIR)/ThreadPool_test : ThreadPool_test.cpp
$(BINS-DIR)/Thread_basetest : Thread_basetest.cpp
$(BINS-DIR)/Thread_advancetest : Thread_advancetest.cpp
$(BINS-DIR)/XmlParser_test : XmlParser_test.cpp

.PHONY : clean
clean :
	rm -f $(BINS)
