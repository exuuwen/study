dpdk_dir=/root/dpdk-16.04/x86_64-native-linuxapp-gcc
REGION=DB_DEF
#
# Library directory and libs.
#
LIBS=-L $(dpdk_dir)/lib              \
     -lprotobuf		      \

#
# Include files directory
#
INCLUDES=-I./include                      \
         -I$(dpdk_dir)/include           \

CC=gcc
CXX=g++

CCFLAGS=-DREGIONS=$(REGION) -D__STDC_VERSION__=199901L -g -m64 -march=native -pthread -W -Wall -Wmissing-declarations -Wpointer-arith -Wcast-qual -Wformat-security -Wundef -Wwrite-strings -Wno-error=unused-but-set-variable -Wno-strict-aliasing --include $(dpdk_dir)/include/rte_config.h -g $(INCLUDES) -march=native -DRTE_MACHINE_CPUFLAG_SSE -DRTE_MACHINE_CPUFLAG_SSE2 -DRTE_MACHINE_CPUFLAG_SSE3 -DRTE_MACHINE_CPUFLAG_SSSE3 -DRTE_MACHINE_CPUFLAG_SSE4_1 -DRTE_MACHINE_CPUFLAG_SSE4_2 -DRTE_MACHINE_CPUFLAG_AES -DRTE_MACHINE_CPUFLAG_PCLMULQDQ -DRTE_MACHINE_CPUFLAG_AVX -DRTE_MACHINE_CPUFLAG_RDRAND -DRTE_MACHINE_CPUFLAG_FSGSBASE -DRTE_MACHINE_CPUFLAG_F16C -DRTE_MACHINE_CPUFLAG_AVX2 -DRTE_COMPILE_TIME_CPUFLAGS=RTE_CPUFLAG_SSE,RTE_CPUFLAG_SSE2,RTE_CPUFLAG_SSE3,RTE_CPUFLAG_SSSE3,RTE_CPUFLAG_SSE4_1,RTE_CPUFLAG_SSE4_2,RTE_CPUFLAG_AES,RTE_CPUFLAG_PCLMULQDQ,RTE_CPUFLAG_AVX,RTE_CPUFLAG_RDRAND,RTE_CPUFLAG_FSGSBASE,RTE_CPUFLAG_F16C,RTE_CPUFLAG_AVX2
CXXFLAGS=-std=gnu++11
LINKFLAGS=-g -m64 -march=native -pthread -std=gnu++11 --include $(dpdk_dir)/include/rte_config.h -g  -m64 -march=native -Wl,-melf_x86_64 -pthread $(LIBS) -Wl,--whole-archive -Wl,-ldpdk -Wl,--start-group -Wl,-lrt -Wl,-lm -Wl,-ldl -Wl,--end-group -Wl,--no-whole-archive -Wl,-rpath,$(dpdk_dir)/lib/ -lev

ccfiles=src/config.c src/init.c src/ipv4_dpi.c src/ipv4_rx.c src/main.c src/socket.c src/stats.c src/task.c src/packet.c 
ccobjs=$(ccfiles:.c=.o)

cxxfiles=src/store.cpp src/ucloud.pb.cpp src/unetanalysis.177000.178000.pb.cpp
cxxobjs=$(cxxfiles:.cpp=.o)

target=build/udpi

ifeq ($(REGION), DB_XJ)
map_file=src/ipaddr_map_country.o
else
map_file=src/ipaddr_map_isp.o
endif


all: $(target)
$(target): $(ccobjs) $(cxxobjs) $(map_file)
	$(CXX) $(LINKFLAGS) $(ccobjs) $(cxxobjs) $(map_file) -o $@
$(ccobjs):%.o:%.c
	$(CC) -c $(CCFLAGS) $< -o $@
$(cxxobjs):%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $(CCFLAGS) $< -o $@

.PHONY : clean
clean :
	rm -f $(target) $(ccobjs) $(cxxobjs)
