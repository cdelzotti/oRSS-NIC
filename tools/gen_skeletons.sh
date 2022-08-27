rm src/bpf/xdp.skel.h
clang -target bpf -g src/bpf/xdp.bpf.c -c -o src/bpf/xdp.o
bpftool gen skeleton src/bpf/xdp.o > src/bpf/xdp.skel.h
rm src/bpf/xdp.o
echo "Skeleton generated in src/bpf/xdp.skel.h"