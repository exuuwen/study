TARGET = tcpconnlat
BPFTOOL ?= bpftool
BPF_OBJ = ${TARGET:=.bpf.o}
USER_C = ${TARGET:=.c}
USER_SKEL = ${TARGET:=.skel.h}

all: $(TARGET) $(BPF_OBJ)
.PHONY: all

CLANG ?= clang
ARCH := $(shell uname -m | sed 's/x86_64/x86/')

%.bpf.o: %.bpf.c vmlinux.h
	$(CLANG) -g -O2 -target bpf -D__TARGET_ARCH_$(ARCH) -I. -o $@ -c $<

$(USER_SKEL): $(BPF_OBJ)
	$(BPFTOOL) gen skeleton $< > $@

$(TARGET): $(USER_C) $(USER_SKEL)
	$(CLANG)  -g -O2 -D__TARGET_ARCH_$(ARCH) -o $(TARGET) $(USER_C) -lelf -lz  -lbpf

clean:
	rm -rf $(BPF_OBJ) $(TARGET) $(USER_SKEL)
