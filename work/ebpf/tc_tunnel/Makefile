build:
	clang -g -O2 -D__x86_64__ -target bpf -c tc-tunnel.c -o tc-tunnel.o
	gcc -O2 -o main main.c  -lbpf -lelf

load: build unload
	tc filter add dev veth0 ingress bpf da obj tc-tunnel.o sec ingress_veth
	tc filter add dev tun ingress bpf da obj tc-tunnel.o sec ingress_gre

unload:
	tc filter del dev veth0 ingress
	tc filter del dev tun ingress
	rm -f /sys/fs/bpf/tc/globals/*

show:
	bpftool map show

clean:
	rm -rf *.o
	rm -rf main

