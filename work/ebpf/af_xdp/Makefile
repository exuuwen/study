build:
	clang -g -O2 -target bpf -c bpf.c -o bpf.o
	gcc -g -O2 -lbpf user.c -o user

load: build unload
	ip link set dev aa xdp obj bpf.o sec xdp

unload:
	 ip link set dev aa xdp off
	 rm -f /sys/fs/bpf/tc/globals/*

show:
	bpftool map show

clean:
	rm -rf bpf.o
	rm -rf user
