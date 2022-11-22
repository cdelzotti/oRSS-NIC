# SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(oRSS)

# Tell cmake where to find BpfObject module
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake)

# set(OVS_PATH ovs)

# Build vendored libbpf
include(ExternalProject)
# ExternalProject_Add(libbpf
#         PREFIX libbpf
#         SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libbpf/src
#         CONFIGURE_COMMAND ""
#         BUILD_COMMAND make
#         BUILD_STATIC_ONLY=1
#         OBJDIR=${CMAKE_CURRENT_BINARY_DIR}/libbpf/libbpf
#         DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/libbpf
#         INCLUDEDIR=
#         LIBDIR=
#         UAPIDIR=
#         install
#         BUILD_IN_SOURCE TRUE
#         INSTALL_COMMAND ""
#         STEP_TARGETS build
#         )

# ExternalProject_Add(ovs
#         PREFIX ovs
#         SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${OVS_PATH}
#         CONFIGURE_COMMAND "./configure"
#         BUILD_COMMAND make
#         BUILD_STATIC_ONLY=1
#         OBJDIR=${CMAKE_CURRENT_BINARY_DIR}/${OVS_PATH}/ovs
#         DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/${OVS_PATH}
#         INCLUDEDIR=
#         LIBDIR=
#         UAPIDIR=
#         install
#         BUILD_IN_SOURCE TRUE
#         INSTALL_COMMAND ":"
#         STEP_TARGETS build
#         )

# add_library(ovs STATIC IMPORTED)
# target_link_libraries(ovs INTERFACE ${OVS_PATH}/lib/)

# Set BpfObject input parameters -- note this is usually not necessary unless
# you're in a highly vendored environment (like libbpf-bootstrap)
# set(BPFOBJECT_BPFTOOL_EXE bpftool)
# set(BPFOBJECT_VMLINUX_H ${CMAKE_CURRENT_SOURCE_DIR}/vmlinux/vmlinux.h)
# set(LIBBPF_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libbpf)
# set(LIBBPF_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libbpf/libbpf.a)
# set(CMAKE_EXE_LINKER_FLAGS)

# find_package(BpfObject REQUIRED)
# find_package(PkgConfig REQUIRED)
# Check DPDK
# pkg_check_modules(DPDK REQUIRED IMPORTED_TARGET libdpdk)

# bpf_object(xdp src/bpf/xdp.bpf.c)
# add_dependencies(xdp_skel libbpf-build)
# add_compile_definitions(DPDK_NETDEV)

add_compile_options(-fpic -ggdb)
add_executable(orss 
src/main.c 
src/env.h 
# src/bpf/xdp.bpf.h 
# src/load_bpf.c src/load_bpf.h
src/ringbuffer.c src/ringbuffer.h
src/hashmap.c src/hashmap.h 
src/balancer.c src/balancer.h
src/openflow.c src/openflow.h)
# add_executable(orss src/main.c src/env.h src/bpf/xdp.bpf.h src/load_bpf.c src/load_bpf.h src/ovs_utils.h src/ovs_utils.c)
# target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs)
# target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OVS_PATH}/ofproto)
# target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${OVS_PATH}/include)
# target_include_directories(orss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ovs/lib)
# target_link_libraries(orss)
