# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(oRSS)

# Tell cmake where to find BpfObject module
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake)

set(OVS_PATH ovs)

# Build vendored libbpf
include(ExternalProject)
ExternalProject_Add(libbpf
        PREFIX libbpf
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libbpf/src
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make
        BUILD_STATIC_ONLY=1
        OBJDIR=${CMAKE_CURRENT_BINARY_DIR}/libbpf/libbpf
        DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/libbpf
        INCLUDEDIR=
        LIBDIR=
        UAPIDIR=
        install
        BUILD_IN_SOURCE TRUE
        INSTALL_COMMAND ""
        STEP_TARGETS build
        )

ExternalProject_Add(ovs
        PREFIX ovs
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${OVS_PATH}
        CONFIGURE_COMMAND "./configure"
        BUILD_COMMAND make
        BUILD_STATIC_ONLY=1
        OBJDIR=${CMAKE_CURRENT_BINARY_DIR}/${OVS_PATH}/ovs
        DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/${OVS_PATH}
        INCLUDEDIR=
        LIBDIR=
        UAPIDIR=
        install
        BUILD_IN_SOURCE TRUE
        INSTALL_COMMAND ":"
        STEP_TARGETS build
        )

# add_library(ovs STATIC IMPORTED)
# target_link_libraries(ovs INTERFACE ${OVS_PATH}/lib/)

# Set BpfObject input parameters -- note this is usually not necessary unless
# you're in a highly vendored environment (like libbpf-bootstrap)
set(BPFOBJECT_BPFTOOL_EXE bpftool)
set(BPFOBJECT_VMLINUX_H ${CMAKE_CURRENT_SOURCE_DIR}/vmlinux/vmlinux.h)
set(LIBBPF_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libbpf)
set(LIBBPF_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libbpf/libbpf.a)
set(CMAKE_EXE_LINKER_FLAGS)

find_package(BpfObject REQUIRED)
find_package(PkgConfig REQUIRED)
# Check DPDK
pkg_check_modules(DPDK REQUIRED IMPORTED_TARGET libdpdk)

bpf_object(xdp src/bpf/xdp.bpf.c)
add_dependencies(xdp_skel libbpf-build)
add_compile_definitions(DPDK_NETDEV)

add_compile_options(-mssse3 -fpic -ggdb)
add_executable(orss 
src/main.c 
src/env.h 
src/bpf/xdp.bpf.h 
src/load_bpf.c src/load_bpf.h
src/ringbuffer.c src/ringbuffer.h
src/hashmap.c src/hashmap.h 
# ${OVS_PATH}/lib/byte-order.h 
# ${OVS_PATH}/lib/flow.h ${OVS_PATH}/lib/flow.c 
# ${OVS_PATH}/lib/classifier.h ${OVS_PATH}/lib/classifier.c
# ${OVS_PATH}/lib/command-line.h ${OVS_PATH}/lib/command-line.c 
# ${OVS_PATH}/lib/daemon-unix.c 
# ${OVS_PATH}/lib/daemon.h ${OVS_PATH}/lib/daemon.c 
# ${OVS_PATH}/lib/colors.h ${OVS_PATH}/lib/colors.c 
# ${OVS_PATH}/lib/pvector.h ${OVS_PATH}/lib/pvector.c  
# ${OVS_PATH}/lib/compiler.h 
# ${OVS_PATH}/lib/dirs.h ${OVS_PATH}/lib/dirs.c 
# ${OVS_PATH}/lib/dp-packet.h ${OVS_PATH}/lib/dp-packet.c 
# ${OVS_PATH}/lib/nx-match.h ${OVS_PATH}/lib/nx-match.c 
# ${OVS_PATH}/lib/timeval.h ${OVS_PATH}/lib/timeval.c 
# ${OVS_PATH}/lib/odp-util.h ${OVS_PATH}/lib/odp-util.c 
# ${OVS_PATH}/lib/ovs-rcu.h ${OVS_PATH}/lib/ovs-rcu.c 
# ${OVS_PATH}/lib/tun-metadata.h ${OVS_PATH}/lib/tun-metadata.c 
# ${OVS_PATH}/lib/ofp-version-opt.h ${OVS_PATH}/lib/ofp-version-opt.c 

${OVS_PATH}/lib/entropy.h 
${OVS_PATH}/lib/entropy.c
${OVS_PATH}/lib/reconnect.h 
${OVS_PATH}/lib/reconnect.c
${OVS_PATH}/lib/fatal-signal.h 
${OVS_PATH}/lib/fatal-signal.c 
${OVS_PATH}/lib/lldp/aa-structs.h
${OVS_PATH}/lib/lldp/lldp.c
${OVS_PATH}/lib/lldp/lldp-const.h
${OVS_PATH}/lib/lldp/lldpd.c
${OVS_PATH}/lib/lldp/lldpd.h
${OVS_PATH}/lib/lldp/lldpd-structs.c
${OVS_PATH}/lib/lldp/lldpd-structs.h
${OVS_PATH}/lib/lldp/lldp-tlv.h

${OVS_PATH}/lib/rstp-state-machines.h
${OVS_PATH}/lib/rstp-state-machines.c
${OVS_PATH}/lib/odp-util.h 
${OVS_PATH}/lib/odp-util.c
${OVS_PATH}/lib/odp-execute.h 
${OVS_PATH}/lib/odp-execute.c
# ${OVS_PATH}/lib/odp-execute-private.h 
# ${OVS_PATH}/lib/odp-execute-private.c 
${OVS_PATH}/lib/aes128.c
${OVS_PATH}/lib/aes128.h
${OVS_PATH}/lib/async-append-aio.c
${OVS_PATH}/lib/async-append.h
# ${OVS_PATH}/lib/async-append-null.c
${OVS_PATH}/lib/backtrace.c
${OVS_PATH}/lib/backtrace.h
${OVS_PATH}/lib/bfd.c
${OVS_PATH}/lib/bfd.h
${OVS_PATH}/lib/bitmap.h
${OVS_PATH}/lib/bundle.c
${OVS_PATH}/lib/bundle.h
${OVS_PATH}/lib/byte-order.h
${OVS_PATH}/lib/byteq.c
${OVS_PATH}/lib/byteq.h
${OVS_PATH}/lib/ccmap.c
${OVS_PATH}/lib/ccmap.h
${OVS_PATH}/lib/cfm.c
${OVS_PATH}/lib/cfm.h
${OVS_PATH}/lib/classifier.c
${OVS_PATH}/lib/classifier.h
${OVS_PATH}/lib/classifier-private.h
${OVS_PATH}/lib/cmap.c
${OVS_PATH}/lib/cmap.h
${OVS_PATH}/lib/colors.c
${OVS_PATH}/lib/colors.h
${OVS_PATH}/lib/command-line.c
${OVS_PATH}/lib/command-line.h
${OVS_PATH}/lib/compiler.h
${OVS_PATH}/lib/connectivity.c
${OVS_PATH}/lib/connectivity.h
${OVS_PATH}/lib/conntrack.c
${OVS_PATH}/lib/conntrack.h
${OVS_PATH}/lib/conntrack-icmp.c
${OVS_PATH}/lib/conntrack-other.c
${OVS_PATH}/lib/conntrack-private.h
${OVS_PATH}/lib/conntrack-tcp.c
${OVS_PATH}/lib/conntrack-tp.c
${OVS_PATH}/lib/conntrack-tp.h
${OVS_PATH}/lib/coverage.c
${OVS_PATH}/lib/coverage.h
${OVS_PATH}/lib/cpu.c
${OVS_PATH}/lib/cpu.h
${OVS_PATH}/lib/crc32c.c
${OVS_PATH}/lib/crc32c.h
${OVS_PATH}/lib/csum.c
${OVS_PATH}/lib/csum.h
${OVS_PATH}/lib/ct-dpif.c
${OVS_PATH}/lib/ct-dpif.h
${OVS_PATH}/lib/daemon.c
${OVS_PATH}/lib/daemon.h
${OVS_PATH}/lib/daemon-private.h
${OVS_PATH}/lib/daemon-unix.c
${OVS_PATH}/lib/db-ctl-base.c
${OVS_PATH}/lib/db-ctl-base.h
${OVS_PATH}/lib/dhcp.h
${OVS_PATH}/lib/dhparams.c
${OVS_PATH}/lib/dhparams.h
${OVS_PATH}/lib/dirs.c
${OVS_PATH}/lib/dirs.h
${OVS_PATH}/lib/dns-resolve.c
${OVS_PATH}/lib/dns-resolve.h
# ${OVS_PATH}/lib/dns-resolve-stub.c
${OVS_PATH}/lib/dpctl.c
${OVS_PATH}/lib/dpctl.h
${OVS_PATH}/lib/netdev-offload.c
${OVS_PATH}/lib/netdev-offload.h
${OVS_PATH}/lib/netdev-offload-provider.h
${OVS_PATH}/lib/netdev-offload-tc.c
${OVS_PATH}/lib/netdev-offload-dpdk.c
# ${OVS_PATH}/lib/dpdk-stub.c
${OVS_PATH}/lib/dpif.c
${OVS_PATH}/lib/dpif.h
# ${OVS_PATH}/lib/dpif-netdev-avx512.c
${OVS_PATH}/lib/dpif-netdev.c
# ${OVS_PATH}/lib/dpif-netdev-extract-avx512.c
${OVS_PATH}/lib/dpif-netdev-extract-study.c
${OVS_PATH}/lib/dpif-netdev.h
${OVS_PATH}/lib/dpif-netdev-lookup-autovalidator.c
# ${OVS_PATH}/lib/dpif-netdev-lookup-avx512-gather.c
${OVS_PATH}/lib/dpif-netdev-lookup.c
${OVS_PATH}/lib/dpif-netdev-lookup-generic.c
${OVS_PATH}/lib/dpif-netdev-lookup.h
${OVS_PATH}/lib/dpif-netdev-perf.c
${OVS_PATH}/lib/dpif-netdev-perf.h
${OVS_PATH}/lib/dpif-netdev-private-dfc.c
${OVS_PATH}/lib/dpif-netdev-private-dfc.h
${OVS_PATH}/lib/dpif-netdev-private-dpcls.h
${OVS_PATH}/lib/dpif-netdev-private-dpif.c
${OVS_PATH}/lib/dpif-netdev-private-dpif.h
${OVS_PATH}/lib/dpif-netdev-private-extract.c
${OVS_PATH}/lib/dpif-netdev-private-extract.h
${OVS_PATH}/lib/dpif-netdev-private-flow.h
${OVS_PATH}/lib/dpif-netdev-private.h
${OVS_PATH}/lib/dpif-netdev-private-thread.h
${OVS_PATH}/lib/dpif-netlink.c
${OVS_PATH}/lib/dpif-netlink.h
${OVS_PATH}/lib/dpif-netlink-rtnl.c
${OVS_PATH}/lib/dpif-netlink-rtnl.h
${OVS_PATH}/lib/dpif-provider.h
${OVS_PATH}/lib/dpdk.c
${OVS_PATH}/lib/dpdk.h
${OVS_PATH}/lib/dp-packet.c
${OVS_PATH}/lib/dp-packet.h
${OVS_PATH}/lib/fat-rwlock.c
${OVS_PATH}/lib/fat-rwlock.h
${OVS_PATH}/lib/flow.c
${OVS_PATH}/lib/flow.h
${OVS_PATH}/lib/getopt_long.c
${OVS_PATH}/lib/guarded-list.c
${OVS_PATH}/lib/guarded-list.h
${OVS_PATH}/lib/hash-aarch64.h
${OVS_PATH}/lib/hash.c
${OVS_PATH}/lib/hash.h
${OVS_PATH}/lib/heap.c
${OVS_PATH}/lib/heap.h
${OVS_PATH}/lib/hindex.c
${OVS_PATH}/lib/hindex.h
${OVS_PATH}/lib/hmap.c
${OVS_PATH}/lib/hmapx.c
${OVS_PATH}/lib/hmapx.h
${OVS_PATH}/lib/id-fpool.c
${OVS_PATH}/lib/id-fpool.h
${OVS_PATH}/lib/id-pool.c
${OVS_PATH}/lib/id-pool.h
# ${OVS_PATH}/lib/if-notifier-bsd.c
${OVS_PATH}/lib/if-notifier.c
${OVS_PATH}/lib/if-notifier.h
${OVS_PATH}/lib/if-notifier-manual.c
# ${OVS_PATH}/lib/if-notifier-stub.c
${OVS_PATH}/lib/ipf.c
${OVS_PATH}/lib/ipf.h
${OVS_PATH}/lib/lacp.h
${OVS_PATH}/lib/lacp.c
${OVS_PATH}/lib/latch.h
${OVS_PATH}/lib/latch-unix.c
${OVS_PATH}/lib/learn.c
${OVS_PATH}/lib/learn.h
${OVS_PATH}/lib/learning-switch.c
${OVS_PATH}/lib/learning-switch.h
${OVS_PATH}/lib/lockfile.c
${OVS_PATH}/lib/lockfile.h
${OVS_PATH}/lib/mac-learning.c
${OVS_PATH}/lib/mac-learning.h
${OVS_PATH}/lib/match.c
${OVS_PATH}/lib/mcast-snooping.c
${OVS_PATH}/lib/mcast-snooping.h
${OVS_PATH}/lib/memory.c
${OVS_PATH}/lib/memory.h
${OVS_PATH}/lib/meta-flow.c
${OVS_PATH}/lib/mov-avg.h
${OVS_PATH}/lib/mpsc-queue.c
${OVS_PATH}/lib/mpsc-queue.h
${OVS_PATH}/lib/multipath.c
${OVS_PATH}/lib/multipath.h
${OVS_PATH}/lib/namemap.c
# ${OVS_PATH}/lib/netdev-afxdp.c
# ${OVS_PATH}/lib/netdev-afxdp.h
# ${OVS_PATH}/lib/netdev-afxdp-pool.c
# ${OVS_PATH}/lib/netdev-afxdp-pool.h
# ${OVS_PATH}/lib/netdev-bsd.c
${OVS_PATH}/lib/netdev.c
${OVS_PATH}/lib/netdev-dpdk.c
${OVS_PATH}/lib/netdev-dpdk.h
${OVS_PATH}/lib/netdev-dummy.c
${OVS_PATH}/lib/netdev.h
${OVS_PATH}/lib/netdev-linux.c
${OVS_PATH}/lib/netdev-linux.h
${OVS_PATH}/lib/netdev-linux-private.h
${OVS_PATH}/lib/netdev-native-tnl.c
${OVS_PATH}/lib/netdev-native-tnl.h
${OVS_PATH}/lib/netdev-provider.h
${OVS_PATH}/lib/netdev-vport.c
${OVS_PATH}/lib/netdev-vport.h
${OVS_PATH}/lib/netdev-vport-private.h
${OVS_PATH}/lib/netflow.h
${OVS_PATH}/lib/netlink.c
${OVS_PATH}/lib/netlink-conntrack.c
${OVS_PATH}/lib/netlink-conntrack.h
${OVS_PATH}/lib/netlink.h
${OVS_PATH}/lib/netlink-notifier.c
${OVS_PATH}/lib/netlink-notifier.h
${OVS_PATH}/lib/netlink-protocol.h
${OVS_PATH}/lib/netlink-socket.c
${OVS_PATH}/lib/netlink-socket.h
${OVS_PATH}/lib/netnsid.h
${OVS_PATH}/lib/nx-match.c
${OVS_PATH}/lib/nx-match.h
${OVS_PATH}/lib/object-collection.c
${OVS_PATH}/lib/object-collection.h
# ${OVS_PATH}/lib/odp-execute-avx512.c-Wl,--as-needed -lrte_node -lrte_graph -lrte_flow_classify -lrte_pipeline -lrte_table -lrte_pdump -lrte_port -lrte_fib -lrte_ipsec -lrte_vhost -lrte_stack -lrte_security -lrte_sched -lrte_reorder -lrte_rib -lrte_dmadev -lrte_regexdev -lrte_rawdev -lrte_power -lrte_pcapng -lrte_member -lrte_lpm -lrte_latencystats -lrte_kni -lrte_jobstats -lrte_ip_frag -lrte_gso -lrte_gro -lrte_gpudev -lrte_eventdev -lrte_efd -lrte_distributor -lrte_cryptodev -lrte_compressdev -lrte_cfgfile -lrte_bpf -lrte_bitratestats -lrte_bbdev -lrte_acl -lrte_timer -lrte_hash -lrte_metrics -lrte_cmdline -lrte_pci -lrte_ethdev -lrte_meter -lrte_net -lrte_mbuf -lrte_mempool -lrte_rcu -lrte_ring -lrte_eal -lrte_telemetry -lrte_kvargs -lbsd 
${OVS_PATH}/lib/ofp-util.c
${OVS_PATH}/lib/ofp-version-opt.c
${OVS_PATH}/lib/ofp-version-opt.h
${OVS_PATH}/lib/ovs-atomic-c11.h
${OVS_PATH}/lib/ovs-atomic-c++.h
${OVS_PATH}/lib/ovs-atomic-clang.h
${OVS_PATH}/lib/ovs-atomic-flag-gcc4.7+.h
${OVS_PATH}/lib/ovs-atomic-gcc4.7+.h
${OVS_PATH}/lib/ovs-atomic-gcc4+.h
${OVS_PATH}/lib/ovs-atomic.h
${OVS_PATH}/lib/ovs-atomic-i586.h
${OVS_PATH}/lib/ovs-atomic-locked.c
${OVS_PATH}/lib/ovs-atomic-locked.h
${OVS_PATH}/lib/ovs-atomic-msvc.h
${OVS_PATH}/lib/ovs-atomic-pthreads.h
${OVS_PATH}/lib/ovs-atomic-x86_64.h
${OVS_PATH}/lib/ovsdb-condition.c
${OVS_PATH}/lib/ovsdb-condition.h
${OVS_PATH}/lib/ovsdb-cs.c
${OVS_PATH}/lib/ovsdb-cs.h
${OVS_PATH}/lib/ovsdb-data.c
${OVS_PATH}/lib/ovsdb-data.h
${OVS_PATH}/lib/ovsdb-error.c
${OVS_PATH}/lib/ovsdb-error.h
${OVS_PATH}/lib/ovsdb-idl.c
${OVS_PATH}/lib/ovsdb-idl.h
${OVS_PATH}/lib/ovsdb-idl-provider.h
${OVS_PATH}/lib/ovsdb-map-op.c
${OVS_PATH}/lib/ovsdb-map-op.h
${OVS_PATH}/lib/ovsdb-parser.c
${OVS_PATH}/lib/ovsdb-parser.h
${OVS_PATH}/lib/ovsdb-server-idl.c
${OVS_PATH}/lib/ovsdb-server-idl.h
${OVS_PATH}/lib/ovsdb-session.c
${OVS_PATH}/lib/ovsdb-session.h
${OVS_PATH}/lib/ovsdb-set-op.c
${OVS_PATH}/lib/ovsdb-set-op.h
${OVS_PATH}/lib/ovsdb-types.c
${OVS_PATH}/lib/ovsdb-types.h
${OVS_PATH}/lib/ovs-lldp.c
${OVS_PATH}/lib/ovs-lldp.h
${OVS_PATH}/lib/ovs-numa.c
${OVS_PATH}/lib/ovs-numa.h
${OVS_PATH}/lib/ovs-rcu.c
${OVS_PATH}/lib/ovs-rcu.h
${OVS_PATH}/lib/ovs-replay.c
${OVS_PATH}/lib/ovs-replay.h
${OVS_PATH}/lib/ovs-router.c
${OVS_PATH}/lib/ovs-router.h
${OVS_PATH}/lib/ovs-thread.c
${OVS_PATH}/lib/ovs-thread.h
${OVS_PATH}/lib/ox-stat.c
${OVS_PATH}/lib/ox-stat.h
${OVS_PATH}/lib/packets.c
${OVS_PATH}/lib/packets.h
${OVS_PATH}/lib/pcap-file.c
${OVS_PATH}/lib/pcap-file.h
${OVS_PATH}/lib/perf-counter.c
${OVS_PATH}/lib/perf-counter.h
${OVS_PATH}/lib/poll-loop.c
${OVS_PATH}/lib/process.c
${OVS_PATH}/lib/process.h
${OVS_PATH}/lib/pvector.c
${OVS_PATH}/lib/pvector.h
${OVS_PATH}/lib/random.c
${OVS_PATH}/lib/random.h
# ${OVS_PATH}/lib/rtbsd.h
${OVS_PATH}/lib/rtnetlink.c
${OVS_PATH}/lib/rtnetlink.h
${OVS_PATH}/lib/sat-math.h
${OVS_PATH}/lib/seq.c
${OVS_PATH}/lib/seq.h
${OVS_PATH}/lib/sflow_agent.c
${OVS_PATH}/lib/sflow_api.h
${OVS_PATH}/lib/sflow.h
${OVS_PATH}/lib/sflow_poller.c
${OVS_PATH}/lib/sflow_receiver.c
${OVS_PATH}/lib/sflow_sampler.c
${OVS_PATH}/lib/sha1.c
${OVS_PATH}/lib/sha1.h
${OVS_PATH}/lib/shash.c
${OVS_PATH}/lib/signals.c
${OVS_PATH}/lib/signals.h
${OVS_PATH}/lib/simap.c
${OVS_PATH}/lib/simap.h
${OVS_PATH}/lib/skiplist.c
${OVS_PATH}/lib/skiplist.h
${OVS_PATH}/lib/smap.c
${OVS_PATH}/lib/smap.h
${OVS_PATH}/lib/socket-util.c
${OVS_PATH}/lib/socket-util.h
${OVS_PATH}/lib/socket-util-unix.c
${OVS_PATH}/lib/sort.c
${OVS_PATH}/lib/sort.h
${OVS_PATH}/lib/sset.c
${OVS_PATH}/lib/sset.h
${OVS_PATH}/lib/stdio.c
${OVS_PATH}/lib/stdio.h
${OVS_PATH}/lib/stdio.h.in
${OVS_PATH}/lib/stopwatch.c
${OVS_PATH}/lib/stopwatch.h
${OVS_PATH}/lib/stp.c
${OVS_PATH}/lib/stp.h
${OVS_PATH}/lib/stream.c
${OVS_PATH}/lib/stream-fd.c
${OVS_PATH}/lib/stream-fd.h
${OVS_PATH}/lib/stream.h
# ${OVS_PATH}/lib/stream-nossl.c
${OVS_PATH}/lib/stream-provider.h
${OVS_PATH}/lib/stream-replay.c
${OVS_PATH}/lib/stream-ssl.c
${OVS_PATH}/lib/stream-ssl.h
${OVS_PATH}/lib/stream-tcp.c
${OVS_PATH}/lib/stream-unix.c
${OVS_PATH}/lib/string.c
${OVS_PATH}/lib/string.h
${OVS_PATH}/lib/string.h.in
${OVS_PATH}/lib/strsep.c
${OVS_PATH}/lib/svec.c
${OVS_PATH}/lib/svec.h
${OVS_PATH}/lib/syslog-direct.c
${OVS_PATH}/lib/syslog-direct.h
${OVS_PATH}/lib/syslog-libc.c
${OVS_PATH}/lib/syslog-libc.h
${OVS_PATH}/lib/syslog-null.c
${OVS_PATH}/lib/syslog-null.h
${OVS_PATH}/lib/syslog-provider.h
${OVS_PATH}/lib/table.c
${OVS_PATH}/lib/table.h
${OVS_PATH}/lib/tc.c
${OVS_PATH}/lib/tc.h
${OVS_PATH}/lib/timer.c
${OVS_PATH}/lib/timer.h
${OVS_PATH}/lib/timeval.c
${OVS_PATH}/lib/timeval.h
${OVS_PATH}/lib/tnl-neigh-cache.c
${OVS_PATH}/lib/tnl-neigh-cache.h
${OVS_PATH}/lib/tnl-ports.c
${OVS_PATH}/lib/tnl-ports.h
${OVS_PATH}/lib/token-bucket.c
${OVS_PATH}/lib/tun-metadata.c
${OVS_PATH}/lib/tun-metadata.h
${OVS_PATH}/lib/unaligned.h
${OVS_PATH}/lib/unicode.c
${OVS_PATH}/lib/unicode.h
${OVS_PATH}/lib/unixctl.c
${OVS_PATH}/lib/unixctl.h
${OVS_PATH}/lib/userspace-tso.c
${OVS_PATH}/lib/userspace-tso.h
${OVS_PATH}/lib/util.c
${OVS_PATH}/lib/util.h
${OVS_PATH}/lib/uuid.c
${OVS_PATH}/lib/uuid.h
${OVS_PATH}/lib/valgrind.h
${OVS_PATH}/lib/vconn.c
${OVS_PATH}/lib/vconn-provider.h
${OVS_PATH}/lib/vconn-stream.c
${OVS_PATH}/lib/versions.h
${OVS_PATH}/lib/vlan-bitmap.c
${OVS_PATH}/lib/vlan-bitmap.h
${OVS_PATH}/lib/vl-mff-map.h
${OVS_PATH}/lib/vlog.c
${OVS_PATH}/lib/vswitch-idl.c
${OVS_PATH}/lib/vswitch-idl.h
${OVS_PATH}/lib/rstp.h
${OVS_PATH}/lib/rstp.c
${OVS_PATH}/lib/route-table.h
${OVS_PATH}/lib/route-table.c
${OVS_PATH}/lib/jhash.h
${OVS_PATH}/lib/jhash.c
# ${OVS_PATH}/lib/wmi.c
# ${OVS_PATH}/lib/wmi.h

${OVS_PATH}/ofproto/bond.c
${OVS_PATH}/ofproto/bond.h
${OVS_PATH}/ofproto/bundles.c
${OVS_PATH}/ofproto/bundles.h
${OVS_PATH}/ofproto/collectors.c
${OVS_PATH}/ofproto/collectors.h
${OVS_PATH}/ofproto/connmgr.c
${OVS_PATH}/ofproto/connmgr.h
${OVS_PATH}/ofproto/fail-open.c
${OVS_PATH}/ofproto/fail-open.h
${OVS_PATH}/ofproto/in-band.c
${OVS_PATH}/ofproto/in-band.h
${OVS_PATH}/ofproto/names.c
${OVS_PATH}/ofproto/netflow.c
${OVS_PATH}/ofproto/netflow.h
${OVS_PATH}/ofproto/ofproto.c
${OVS_PATH}/ofproto/ofproto-dpif.c
${OVS_PATH}/ofproto/ofproto-dpif-mirror.h
${OVS_PATH}/ofproto/ofproto-dpif-mirror.c
${OVS_PATH}/ofproto/ofproto-dpif-monitor.h
${OVS_PATH}/ofproto/ofproto-dpif-monitor.c
${OVS_PATH}/ofproto/ofproto-dpif-sflow.h
${OVS_PATH}/ofproto/ofproto-dpif-sflow.c
${OVS_PATH}/ofproto/ofproto-dpif-rid.h
${OVS_PATH}/ofproto/ofproto-dpif-rid.c
${OVS_PATH}/ofproto/ofproto-dpif-upcall.h
${OVS_PATH}/ofproto/ofproto-dpif-upcall.c
${OVS_PATH}/ofproto/ofproto-dpif-trace.h
${OVS_PATH}/ofproto/ofproto-dpif-trace.c
${OVS_PATH}/ofproto/ofproto-dpif-ipfix.h
${OVS_PATH}/ofproto/ofproto-dpif-ipfix.c
${OVS_PATH}/ofproto/ofproto-dpif-xlate.c
${OVS_PATH}/ofproto/ofproto-dpif-xlate-cache.c
${OVS_PATH}/ofproto/ofproto-dpif-xlate-cache.h
${OVS_PATH}/ofproto/ofproto-dpif-xlate.h
${OVS_PATH}/ofproto/ofproto.h
${OVS_PATH}/ofproto/ofproto-provider.h
${OVS_PATH}/ofproto/pinsched.c
${OVS_PATH}/ofproto/pinsched.h
${OVS_PATH}/ofproto/tunnel.c
${OVS_PATH}/ofproto/tunnel.h

${OVS_PATH}/ofproto/ofproto.h ${OVS_PATH}/ofproto/ofproto.c
${OVS_PATH}/ofproto/names.c
${OVS_PATH}/include/openflow/nicira-ext.h
${OVS_PATH}/include/openflow/openflow.h
${OVS_PATH}/include/openvswitch/thread.h
${OVS_PATH}/include/openvswitch/dynamic-string.h ${OVS_PATH}/lib/dynamic-string.c
${OVS_PATH}/include/openvswitch/meta-flow.h ${OVS_PATH}/lib/meta-flow.c
${OVS_PATH}/include/openvswitch/ofp-actions.h ${OVS_PATH}/lib/ofp-actions.c
${OVS_PATH}/include/openvswitch/ofp-packet.h ${OVS_PATH}/lib/ofp-packet.c
${OVS_PATH}/include/openvswitch/ofp-bundle.h ${OVS_PATH}/lib/ofp-bundle.c
${OVS_PATH}/include/openvswitch/ofp-errors.h ${OVS_PATH}/lib/ofp-errors.c
${OVS_PATH}/include/openvswitch/ofp-group.h ${OVS_PATH}/lib/ofp-group.c
${OVS_PATH}/include/openvswitch/ofp-match.h ${OVS_PATH}/lib/ofp-match.c
${OVS_PATH}/include/openvswitch/ofp-meter.h ${OVS_PATH}/lib/ofp-meter.c
${OVS_PATH}/include/openvswitch/ofp-msgs.h ${OVS_PATH}/lib/ofp-msgs.c
${OVS_PATH}/include/openvswitch/ofp-monitor.h ${OVS_PATH}/lib/ofp-monitor.c
${OVS_PATH}/include/openvswitch/ofp-port.h ${OVS_PATH}/lib/ofp-port.c
${OVS_PATH}/include/openvswitch/ofp-print.h ${OVS_PATH}/lib/ofp-print.c
${OVS_PATH}/include/openvswitch/ofp-util.h ${OVS_PATH}/lib/ofp-util.c
${OVS_PATH}/include/openvswitch/ofp-parse.h ${OVS_PATH}/lib/ofp-parse.c
${OVS_PATH}/include/openvswitch/ofp-queue.h ${OVS_PATH}/lib/ofp-queue.c
${OVS_PATH}/include/openvswitch/ofp-switch.h ${OVS_PATH}/lib/ofp-switch.c
${OVS_PATH}/include/openvswitch/ofp-table.h ${OVS_PATH}/lib/ofp-table.c
${OVS_PATH}/include/openvswitch/ofp-flow.h ${OVS_PATH}/lib/ofp-flow.c
${OVS_PATH}/include/openvswitch/ofp-protocol.h ${OVS_PATH}/lib/ofp-protocol.c
${OVS_PATH}/include/openvswitch/ofp-ipfix.h ${OVS_PATH}/lib/ofp-ipfix.c
${OVS_PATH}/include/openvswitch/ofp-connection.h ${OVS_PATH}/lib/ofp-connection.c
${OVS_PATH}/include/openvswitch/ofp-ed-props.h ${OVS_PATH}/lib/ofp-ed-props.c
${OVS_PATH}/include/openvswitch/ofpbuf.h ${OVS_PATH}/lib/ofpbuf.c
${OVS_PATH}/include/openvswitch/shash.h ${OVS_PATH}/lib/shash.c
${OVS_PATH}/include/openvswitch/vconn.h ${OVS_PATH}/lib/vconn.c
${OVS_PATH}/include/openvswitch/vlog.h ${OVS_PATH}/lib/vlog.c
${OVS_PATH}/include/openvswitch/match.h ${OVS_PATH}/lib/match.c
${OVS_PATH}/include/openvswitch/packets.h ${OVS_PATH}/lib/packets.c
${OVS_PATH}/include/openvswitch/json.h ${OVS_PATH}/lib/json.c
${OVS_PATH}/include/openvswitch/ofp-prop.h ${OVS_PATH}/lib/ofp-prop.c
${OVS_PATH}/include/openvswitch/rconn.h ${OVS_PATH}/lib/rconn.c

# ${OVS_PATH}/lib/svec.h ${OVS_PATH}/lib/svec.c
# ${OVS_PATH}/lib/pcap-file.h ${OVS_PATH}/lib/pcap-file.c 
# ${OVS_PATH}/include/openvswitch/poll-loop.h ${OVS_PATH}/lib/poll-loop.c
# ${OVS_PATH}/lib/random.h ${OVS_PATH}/lib/random.c
# ${OVS_PATH}/lib/sort.h ${OVS_PATH}/lib/sort.c
# ${OVS_PATH}/lib/dhparams.h ${OVS_PATH}/lib/dhparams.c
# ${OVS_PATH}/lib/dns-resolve.h ${OVS_PATH}/lib/dns-resolve.c
# ${OVS_PATH}/lib/ovs-thread.h ${OVS_PATH}/lib/ovs-thread.c
${OVS_PATH}/lib/jsonrpc.h ${OVS_PATH}/lib/jsonrpc.c
# ${OVS_PATH}/lib/coverage.h ${OVS_PATH}/lib/coverage.c
# ${OVS_PATH}/lib/socket-util.h ${OVS_PATH}/lib/socket-util-unix.c
# ${OVS_PATH}/lib/stream-unix.c
# ${OVS_PATH}/lib/stream-tcp.c
# ${OVS_PATH}/lib/stream-fd.h ${OVS_PATH}/lib/stream-fd.c
# ${OVS_PATH}/lib/stream-provider.h
# ${OVS_PATH}/lib/stream-replay.c
# ${OVS_PATH}/lib/ovs-replay.h ${OVS_PATH}/lib/ovs-replay.c
# ${OVS_PATH}/lib/stream.h ${OVS_PATH}/lib/stream.c
# ${OVS_PATH}/lib/entropy.h ${OVS_PATH}/lib/entropy.c
# ${OVS_PATH}/lib/stream-ssl.h ${OVS_PATH}/lib/stream-ssl.c
# ${OVS_PATH}/lib/socket-util.h ${OVS_PATH}/lib/socket-util.c
# ${OVS_PATH}/lib/timeval.h ${OVS_PATH}/lib/socket-util.c
# ${OVS_PATH}/lib/unixctl.h ${OVS_PATH}/lib/unixctl.c
# ${OVS_PATH}/lib/util.h ${OVS_PATH}/lib/util.c
src/ovs-ofctl.c)
# add_executable(orss src/main.c src/env.h src/bpf/xdp.bpf.h src/load_bpf.c src/load_bpf.h src/ovs_utils.h src/ovs_utils.c)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OVS_PATH}/ofproto)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OVS_PATH}/include)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs/lib)
target_link_libraries(orss xdp_skel ssl crypto atomic m unwind cap-ng unbound PkgConfig::DPDK)