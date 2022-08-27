#/bin/sh

mkdir ../vmlinux
bpftool btf dump file ${1:-/sys/kernel/btf/vmlinux} format c > ../vmlinux/vmlinux.h
