# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(oRSS)

# Tell cmake where to find BpfObject module
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake)

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
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ovs
        CONFIGURE_COMMAND "./configure"
        BUILD_COMMAND make
        BUILD_STATIC_ONLY=1
        OBJDIR=${CMAKE_CURRENT_BINARY_DIR}/ovs/ovs
        DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/ovs
        INCLUDEDIR=
        LIBDIR=
        UAPIDIR=
        install
        BUILD_IN_SOURCE TRUE
        INSTALL_COMMAND ":"
        STEP_TARGETS build
        )

# add_library(ovs STATIC IMPORTED)
# target_link_libraries(ovs INTERFACE ovs/lib/)

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
src/main.c src/env.h src/bpf/xdp.bpf.h src/load_bpf.c src/load_bpf.h 
# ovs/lib/byte-order.h 
# ovs/lib/flow.h ovs/lib/flow.c 
# ovs/lib/classifier.h ovs/lib/classifier.c
# ovs/lib/command-line.h ovs/lib/command-line.c 
# ovs/lib/daemon-unix.c 
# ovs/lib/daemon.h ovs/lib/daemon.c 
# ovs/lib/colors.h ovs/lib/colors.c 
# ovs/lib/pvector.h ovs/lib/pvector.c  
# ovs/lib/compiler.h 
# ovs/lib/dirs.h ovs/lib/dirs.c 
# ovs/lib/dp-packet.h ovs/lib/dp-packet.c 
# ovs/lib/nx-match.h ovs/lib/nx-match.c 
# ovs/lib/timeval.h ovs/lib/timeval.c 
# ovs/lib/odp-util.h ovs/lib/odp-util.c 
# ovs/lib/ovs-rcu.h ovs/lib/ovs-rcu.c 
# ovs/lib/tun-metadata.h ovs/lib/tun-metadata.c 
# ovs/lib/ofp-version-opt.h ovs/lib/ofp-version-opt.c 

ovs/lib/entropy.h 
ovs/lib/entropy.c
ovs/lib/reconnect.h 
ovs/lib/reconnect.c
ovs/lib/fatal-signal.h 
ovs/lib/fatal-signal.c 
ovs/lib/lldp/aa-structs.h
ovs/lib/lldp/lldp.c
ovs/lib/lldp/lldp-const.h
ovs/lib/lldp/lldpd.c
ovs/lib/lldp/lldpd.h
ovs/lib/lldp/lldpd-structs.c
ovs/lib/lldp/lldpd-structs.h
ovs/lib/lldp/lldp-tlv.h

ovs/lib/rstp-state-machines.h
ovs/lib/rstp-state-machines.c
ovs/lib/odp-util.h 
ovs/lib/odp-util.c
ovs/lib/odp-execute.h 
ovs/lib/odp-execute.c
# ovs/lib/odp-execute-private.h 
# ovs/lib/odp-execute-private.c 
ovs/lib/aes128.c
ovs/lib/aes128.h
ovs/lib/async-append-aio.c
ovs/lib/async-append.h
# ovs/lib/async-append-null.c
ovs/lib/backtrace.c
ovs/lib/backtrace.h
ovs/lib/bfd.c
ovs/lib/bfd.h
ovs/lib/bitmap.h
ovs/lib/bundle.c
ovs/lib/bundle.h
ovs/lib/byte-order.h
ovs/lib/byteq.c
ovs/lib/byteq.h
ovs/lib/ccmap.c
ovs/lib/ccmap.h
ovs/lib/cfm.c
ovs/lib/cfm.h
ovs/lib/classifier.c
ovs/lib/classifier.h
ovs/lib/classifier-private.h
ovs/lib/cmap.c
ovs/lib/cmap.h
ovs/lib/colors.c
ovs/lib/colors.h
ovs/lib/command-line.c
ovs/lib/command-line.h
ovs/lib/compiler.h
ovs/lib/connectivity.c
ovs/lib/connectivity.h
ovs/lib/conntrack.c
ovs/lib/conntrack.h
ovs/lib/conntrack-icmp.c
ovs/lib/conntrack-other.c
ovs/lib/conntrack-private.h
ovs/lib/conntrack-tcp.c
ovs/lib/conntrack-tp.c
ovs/lib/conntrack-tp.h
ovs/lib/coverage.c
ovs/lib/coverage.h
ovs/lib/cpu.c
ovs/lib/cpu.h
ovs/lib/crc32c.c
ovs/lib/crc32c.h
ovs/lib/csum.c
ovs/lib/csum.h
ovs/lib/ct-dpif.c
ovs/lib/ct-dpif.h
ovs/lib/daemon.c
ovs/lib/daemon.h
ovs/lib/daemon-private.h
ovs/lib/daemon-unix.c
ovs/lib/db-ctl-base.c
ovs/lib/db-ctl-base.h
ovs/lib/dhcp.h
ovs/lib/dhparams.c
ovs/lib/dhparams.h
ovs/lib/dirs.c
ovs/lib/dirs.h
ovs/lib/dns-resolve.c
ovs/lib/dns-resolve.h
# ovs/lib/dns-resolve-stub.c
ovs/lib/dpctl.c
ovs/lib/dpctl.h
ovs/lib/netdev-offload.c
ovs/lib/netdev-offload.h
ovs/lib/netdev-offload-provider.h
ovs/lib/netdev-offload-tc.c
ovs/lib/netdev-offload-dpdk.c
# ovs/lib/dpdk-stub.c
ovs/lib/dpif.c
ovs/lib/dpif.h
# ovs/lib/dpif-netdev-avx512.c
ovs/lib/dpif-netdev.c
# ovs/lib/dpif-netdev-extract-avx512.c
ovs/lib/dpif-netdev-extract-study.c
ovs/lib/dpif-netdev.h
ovs/lib/dpif-netdev-lookup-autovalidator.c
# ovs/lib/dpif-netdev-lookup-avx512-gather.c
ovs/lib/dpif-netdev-lookup.c
ovs/lib/dpif-netdev-lookup-generic.c
ovs/lib/dpif-netdev-lookup.h
ovs/lib/dpif-netdev-perf.c
ovs/lib/dpif-netdev-perf.h
ovs/lib/dpif-netdev-private-dfc.c
ovs/lib/dpif-netdev-private-dfc.h
ovs/lib/dpif-netdev-private-dpcls.h
ovs/lib/dpif-netdev-private-dpif.c
ovs/lib/dpif-netdev-private-dpif.h
ovs/lib/dpif-netdev-private-extract.c
ovs/lib/dpif-netdev-private-extract.h
ovs/lib/dpif-netdev-private-flow.h
ovs/lib/dpif-netdev-private.h
ovs/lib/dpif-netdev-private-thread.h
ovs/lib/dpif-netlink.c
ovs/lib/dpif-netlink.h
ovs/lib/dpif-netlink-rtnl.c
ovs/lib/dpif-netlink-rtnl.h
ovs/lib/dpif-provider.h
ovs/lib/dpdk.c
ovs/lib/dpdk.h
ovs/lib/dp-packet.c
ovs/lib/dp-packet.h
ovs/lib/fat-rwlock.c
ovs/lib/fat-rwlock.h
ovs/lib/flow.c
ovs/lib/flow.h
ovs/lib/getopt_long.c
ovs/lib/guarded-list.c
ovs/lib/guarded-list.h
ovs/lib/hash-aarch64.h
ovs/lib/hash.c
ovs/lib/hash.h
ovs/lib/heap.c
ovs/lib/heap.h
ovs/lib/hindex.c
ovs/lib/hindex.h
ovs/lib/hmap.c
ovs/lib/hmapx.c
ovs/lib/hmapx.h
ovs/lib/id-fpool.c
ovs/lib/id-fpool.h
ovs/lib/id-pool.c
ovs/lib/id-pool.h
# ovs/lib/if-notifier-bsd.c
ovs/lib/if-notifier.c
ovs/lib/if-notifier.h
ovs/lib/if-notifier-manual.c
# ovs/lib/if-notifier-stub.c
ovs/lib/ipf.c
ovs/lib/ipf.h
ovs/lib/lacp.h
ovs/lib/lacp.c
ovs/lib/latch.h
ovs/lib/latch-unix.c
ovs/lib/learn.c
ovs/lib/learn.h
ovs/lib/learning-switch.c
ovs/lib/learning-switch.h
ovs/lib/lockfile.c
ovs/lib/lockfile.h
ovs/lib/mac-learning.c
ovs/lib/mac-learning.h
ovs/lib/match.c
ovs/lib/mcast-snooping.c
ovs/lib/mcast-snooping.h
ovs/lib/memory.c
ovs/lib/memory.h
ovs/lib/meta-flow.c
ovs/lib/mov-avg.h
ovs/lib/mpsc-queue.c
ovs/lib/mpsc-queue.h
ovs/lib/multipath.c
ovs/lib/multipath.h
ovs/lib/namemap.c
# ovs/lib/netdev-afxdp.c
# ovs/lib/netdev-afxdp.h
# ovs/lib/netdev-afxdp-pool.c
# ovs/lib/netdev-afxdp-pool.h
# ovs/lib/netdev-bsd.c
ovs/lib/netdev.c
ovs/lib/netdev-dpdk.c
ovs/lib/netdev-dpdk.h
ovs/lib/netdev-dummy.c
ovs/lib/netdev.h
ovs/lib/netdev-linux.c
ovs/lib/netdev-linux.h
ovs/lib/netdev-linux-private.h
ovs/lib/netdev-native-tnl.c
ovs/lib/netdev-native-tnl.h
ovs/lib/netdev-provider.h
ovs/lib/netdev-vport.c
ovs/lib/netdev-vport.h
ovs/lib/netdev-vport-private.h
ovs/lib/netflow.h
ovs/lib/netlink.c
ovs/lib/netlink-conntrack.c
ovs/lib/netlink-conntrack.h
ovs/lib/netlink.h
ovs/lib/netlink-notifier.c
ovs/lib/netlink-notifier.h
ovs/lib/netlink-protocol.h
ovs/lib/netlink-socket.c
ovs/lib/netlink-socket.h
ovs/lib/netnsid.h
ovs/lib/nx-match.c
ovs/lib/nx-match.h
ovs/lib/object-collection.c
ovs/lib/object-collection.h
# ovs/lib/odp-execute-avx512.c-Wl,--as-needed -lrte_node -lrte_graph -lrte_flow_classify -lrte_pipeline -lrte_table -lrte_pdump -lrte_port -lrte_fib -lrte_ipsec -lrte_vhost -lrte_stack -lrte_security -lrte_sched -lrte_reorder -lrte_rib -lrte_dmadev -lrte_regexdev -lrte_rawdev -lrte_power -lrte_pcapng -lrte_member -lrte_lpm -lrte_latencystats -lrte_kni -lrte_jobstats -lrte_ip_frag -lrte_gso -lrte_gro -lrte_gpudev -lrte_eventdev -lrte_efd -lrte_distributor -lrte_cryptodev -lrte_compressdev -lrte_cfgfile -lrte_bpf -lrte_bitratestats -lrte_bbdev -lrte_acl -lrte_timer -lrte_hash -lrte_metrics -lrte_cmdline -lrte_pci -lrte_ethdev -lrte_meter -lrte_net -lrte_mbuf -lrte_mempool -lrte_rcu -lrte_ring -lrte_eal -lrte_telemetry -lrte_kvargs -lbsd 
ovs/lib/ofp-util.c
ovs/lib/ofp-version-opt.c
ovs/lib/ofp-version-opt.h
ovs/lib/ovs-atomic-c11.h
ovs/lib/ovs-atomic-c++.h
ovs/lib/ovs-atomic-clang.h
ovs/lib/ovs-atomic-flag-gcc4.7+.h
ovs/lib/ovs-atomic-gcc4.7+.h
ovs/lib/ovs-atomic-gcc4+.h
ovs/lib/ovs-atomic.h
ovs/lib/ovs-atomic-i586.h
ovs/lib/ovs-atomic-locked.c
ovs/lib/ovs-atomic-locked.h
ovs/lib/ovs-atomic-msvc.h
ovs/lib/ovs-atomic-pthreads.h
ovs/lib/ovs-atomic-x86_64.h
ovs/lib/ovsdb-condition.c
ovs/lib/ovsdb-condition.h
ovs/lib/ovsdb-cs.c
ovs/lib/ovsdb-cs.h
ovs/lib/ovsdb-data.c
ovs/lib/ovsdb-data.h
ovs/lib/ovsdb-error.c
ovs/lib/ovsdb-error.h
ovs/lib/ovsdb-idl.c
ovs/lib/ovsdb-idl.h
ovs/lib/ovsdb-idl-provider.h
ovs/lib/ovsdb-map-op.c
ovs/lib/ovsdb-map-op.h
ovs/lib/ovsdb-parser.c
ovs/lib/ovsdb-parser.h
ovs/lib/ovsdb-server-idl.c
ovs/lib/ovsdb-server-idl.h
ovs/lib/ovsdb-session.c
ovs/lib/ovsdb-session.h
ovs/lib/ovsdb-set-op.c
ovs/lib/ovsdb-set-op.h
ovs/lib/ovsdb-types.c
ovs/lib/ovsdb-types.h
ovs/lib/ovs-lldp.c
ovs/lib/ovs-lldp.h
ovs/lib/ovs-numa.c
ovs/lib/ovs-numa.h
ovs/lib/ovs-rcu.c
ovs/lib/ovs-rcu.h
ovs/lib/ovs-replay.c
ovs/lib/ovs-replay.h
ovs/lib/ovs-router.c
ovs/lib/ovs-router.h
ovs/lib/ovs-thread.c
ovs/lib/ovs-thread.h
ovs/lib/ox-stat.c
ovs/lib/ox-stat.h
ovs/lib/packets.c
ovs/lib/packets.h
ovs/lib/pcap-file.c
ovs/lib/pcap-file.h
ovs/lib/perf-counter.c
ovs/lib/perf-counter.h
ovs/lib/poll-loop.c
ovs/lib/process.c
ovs/lib/process.h
ovs/lib/pvector.c
ovs/lib/pvector.h
ovs/lib/random.c
ovs/lib/random.h
# ovs/lib/rtbsd.h
ovs/lib/rtnetlink.c
ovs/lib/rtnetlink.h
ovs/lib/sat-math.h
ovs/lib/seq.c
ovs/lib/seq.h
ovs/lib/sflow_agent.c
ovs/lib/sflow_api.h
ovs/lib/sflow.h
ovs/lib/sflow_poller.c
ovs/lib/sflow_receiver.c
ovs/lib/sflow_sampler.c
ovs/lib/sha1.c
ovs/lib/sha1.h
ovs/lib/shash.c
ovs/lib/signals.c
ovs/lib/signals.h
ovs/lib/simap.c
ovs/lib/simap.h
ovs/lib/skiplist.c
ovs/lib/skiplist.h
ovs/lib/smap.c
ovs/lib/smap.h
ovs/lib/socket-util.c
ovs/lib/socket-util.h
ovs/lib/socket-util-unix.c
ovs/lib/sort.c
ovs/lib/sort.h
ovs/lib/sset.c
ovs/lib/sset.h
ovs/lib/stdio.c
ovs/lib/stdio.h
ovs/lib/stdio.h.in
ovs/lib/stopwatch.c
ovs/lib/stopwatch.h
ovs/lib/stp.c
ovs/lib/stp.h
ovs/lib/stream.c
ovs/lib/stream-fd.c
ovs/lib/stream-fd.h
ovs/lib/stream.h
# ovs/lib/stream-nossl.c
ovs/lib/stream-provider.h
ovs/lib/stream-replay.c
ovs/lib/stream-ssl.c
ovs/lib/stream-ssl.h
ovs/lib/stream-tcp.c
ovs/lib/stream-unix.c
ovs/lib/string.c
ovs/lib/string.h
ovs/lib/string.h.in
ovs/lib/strsep.c
ovs/lib/svec.c
ovs/lib/svec.h
ovs/lib/syslog-direct.c
ovs/lib/syslog-direct.h
ovs/lib/syslog-libc.c
ovs/lib/syslog-libc.h
ovs/lib/syslog-null.c
ovs/lib/syslog-null.h
ovs/lib/syslog-provider.h
ovs/lib/table.c
ovs/lib/table.h
ovs/lib/tc.c
ovs/lib/tc.h
ovs/lib/timer.c
ovs/lib/timer.h
ovs/lib/timeval.c
ovs/lib/timeval.h
ovs/lib/tnl-neigh-cache.c
ovs/lib/tnl-neigh-cache.h
ovs/lib/tnl-ports.c
ovs/lib/tnl-ports.h
ovs/lib/token-bucket.c
ovs/lib/tun-metadata.c
ovs/lib/tun-metadata.h
ovs/lib/unaligned.h
ovs/lib/unicode.c
ovs/lib/unicode.h
ovs/lib/unixctl.c
ovs/lib/unixctl.h
ovs/lib/userspace-tso.c
ovs/lib/userspace-tso.h
ovs/lib/util.c
ovs/lib/util.h
ovs/lib/uuid.c
ovs/lib/uuid.h
ovs/lib/valgrind.h
ovs/lib/vconn.c
ovs/lib/vconn-provider.h
ovs/lib/vconn-stream.c
ovs/lib/versions.h
ovs/lib/vlan-bitmap.c
ovs/lib/vlan-bitmap.h
ovs/lib/vl-mff-map.h
ovs/lib/vlog.c
ovs/lib/vswitch-idl.c
ovs/lib/vswitch-idl.h
ovs/lib/rstp.h
ovs/lib/rstp.c
ovs/lib/route-table.h
ovs/lib/route-table.c
ovs/lib/jhash.h
ovs/lib/jhash.c
# ovs/lib/wmi.c
# ovs/lib/wmi.h

ovs/ofproto/bond.c
ovs/ofproto/bond.h
ovs/ofproto/bundles.c
ovs/ofproto/bundles.h
ovs/ofproto/collectors.c
ovs/ofproto/collectors.h
ovs/ofproto/connmgr.c
ovs/ofproto/connmgr.h
ovs/ofproto/fail-open.c
ovs/ofproto/fail-open.h
ovs/ofproto/in-band.c
ovs/ofproto/in-band.h
ovs/ofproto/names.c
ovs/ofproto/netflow.c
ovs/ofproto/netflow.h
ovs/ofproto/ofproto.c
ovs/ofproto/ofproto-dpif.c
ovs/ofproto/ofproto-dpif-mirror.h
ovs/ofproto/ofproto-dpif-mirror.c
ovs/ofproto/ofproto-dpif-monitor.h
ovs/ofproto/ofproto-dpif-monitor.c
ovs/ofproto/ofproto-dpif-sflow.h
ovs/ofproto/ofproto-dpif-sflow.c
ovs/ofproto/ofproto-dpif-rid.h
ovs/ofproto/ofproto-dpif-rid.c
ovs/ofproto/ofproto-dpif-upcall.h
ovs/ofproto/ofproto-dpif-upcall.c
ovs/ofproto/ofproto-dpif-trace.h
ovs/ofproto/ofproto-dpif-trace.c
ovs/ofproto/ofproto-dpif-ipfix.h
ovs/ofproto/ofproto-dpif-ipfix.c
ovs/ofproto/ofproto-dpif-xlate.c
ovs/ofproto/ofproto-dpif-xlate-cache.c
ovs/ofproto/ofproto-dpif-xlate-cache.h
ovs/ofproto/ofproto-dpif-xlate.h
ovs/ofproto/ofproto.h
ovs/ofproto/ofproto-provider.h
ovs/ofproto/pinsched.c
ovs/ofproto/pinsched.h
ovs/ofproto/tunnel.c
ovs/ofproto/tunnel.h

ovs/ofproto/ofproto.h ovs/ofproto/ofproto.c
ovs/ofproto/names.c
ovs/include/openflow/nicira-ext.h
ovs/include/openflow/openflow.h
ovs/include/openvswitch/thread.h
ovs/include/openvswitch/dynamic-string.h ovs/lib/dynamic-string.c
ovs/include/openvswitch/meta-flow.h ovs/lib/meta-flow.c
ovs/include/openvswitch/ofp-actions.h ovs/lib/ofp-actions.c
ovs/include/openvswitch/ofp-packet.h ovs/lib/ofp-packet.c
ovs/include/openvswitch/ofp-bundle.h ovs/lib/ofp-bundle.c
ovs/include/openvswitch/ofp-errors.h ovs/lib/ofp-errors.c
ovs/include/openvswitch/ofp-group.h ovs/lib/ofp-group.c
ovs/include/openvswitch/ofp-match.h ovs/lib/ofp-match.c
ovs/include/openvswitch/ofp-meter.h ovs/lib/ofp-meter.c
ovs/include/openvswitch/ofp-msgs.h ovs/lib/ofp-msgs.c
ovs/include/openvswitch/ofp-monitor.h ovs/lib/ofp-monitor.c
ovs/include/openvswitch/ofp-port.h ovs/lib/ofp-port.c
ovs/include/openvswitch/ofp-print.h ovs/lib/ofp-print.c
ovs/include/openvswitch/ofp-util.h ovs/lib/ofp-util.c
ovs/include/openvswitch/ofp-parse.h ovs/lib/ofp-parse.c
ovs/include/openvswitch/ofp-queue.h ovs/lib/ofp-queue.c
ovs/include/openvswitch/ofp-switch.h ovs/lib/ofp-switch.c
ovs/include/openvswitch/ofp-table.h ovs/lib/ofp-table.c
ovs/include/openvswitch/ofp-flow.h ovs/lib/ofp-flow.c
ovs/include/openvswitch/ofp-protocol.h ovs/lib/ofp-protocol.c
ovs/include/openvswitch/ofp-ipfix.h ovs/lib/ofp-ipfix.c
ovs/include/openvswitch/ofp-connection.h ovs/lib/ofp-connection.c
ovs/include/openvswitch/ofp-ed-props.h ovs/lib/ofp-ed-props.c
ovs/include/openvswitch/ofpbuf.h ovs/lib/ofpbuf.c
ovs/include/openvswitch/shash.h ovs/lib/shash.c
ovs/include/openvswitch/vconn.h ovs/lib/vconn.c
ovs/include/openvswitch/vlog.h ovs/lib/vlog.c
ovs/include/openvswitch/match.h ovs/lib/match.c
ovs/include/openvswitch/packets.h ovs/lib/packets.c
ovs/include/openvswitch/json.h ovs/lib/json.c
ovs/include/openvswitch/ofp-prop.h ovs/lib/ofp-prop.c
ovs/include/openvswitch/rconn.h ovs/lib/rconn.c

# ovs/lib/svec.h ovs/lib/svec.c
# ovs/lib/pcap-file.h ovs/lib/pcap-file.c 
# ovs/include/openvswitch/poll-loop.h ovs/lib/poll-loop.c
# ovs/lib/random.h ovs/lib/random.c
# ovs/lib/sort.h ovs/lib/sort.c
# ovs/lib/dhparams.h ovs/lib/dhparams.c
# ovs/lib/dns-resolve.h ovs/lib/dns-resolve.c
# ovs/lib/ovs-thread.h ovs/lib/ovs-thread.c
ovs/lib/jsonrpc.h ovs/lib/jsonrpc.c
# ovs/lib/coverage.h ovs/lib/coverage.c
# ovs/lib/socket-util.h ovs/lib/socket-util-unix.c
# ovs/lib/stream-unix.c
# ovs/lib/stream-tcp.c
# ovs/lib/stream-fd.h ovs/lib/stream-fd.c
# ovs/lib/stream-provider.h
# ovs/lib/stream-replay.c
# ovs/lib/ovs-replay.h ovs/lib/ovs-replay.c
# ovs/lib/stream.h ovs/lib/stream.c
# ovs/lib/entropy.h ovs/lib/entropy.c
# ovs/lib/stream-ssl.h ovs/lib/stream-ssl.c
# ovs/lib/socket-util.h ovs/lib/socket-util.c
# ovs/lib/timeval.h ovs/lib/socket-util.c
# ovs/lib/unixctl.h ovs/lib/unixctl.c
# ovs/lib/util.h ovs/lib/util.c
src/ovs-ofctl.c)
# add_executable(orss src/main.c src/env.h src/bpf/xdp.bpf.h src/load_bpf.c src/load_bpf.h src/ovs_utils.h src/ovs_utils.c)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs/ofproto)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs/include)
target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs/lib)
target_link_libraries(orss xdp_skel ssl crypto atomic m unwind cap-ng unbound PkgConfig::DPDK)