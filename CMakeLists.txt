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

# Set BpfObject input parameters -- note this is usually not necessary unless
# you're in a highly vendored environment (like libbpf-bootstrap)
set(BPFOBJECT_BPFTOOL_EXE bpftool)
set(BPFOBJECT_VMLINUX_H ${CMAKE_CURRENT_SOURCE_DIR}/vmlinux/vmlinux.h)
set(LIBBPF_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libbpf)
set(LIBBPF_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libbpf/libbpf.a)
find_package(BpfObject REQUIRED)

bpf_object(xdp src/bpf/xdp.bpf.c)
add_dependencies(xdp_skel libbpf-build)
add_executable(orss src/main.c src/env.h src/bpf/xdp.bpf.h src/load_bpf.c src/load_bpf.h)
target_link_libraries(orss xdp_skel)
# target_compile_options(program PRIVATE --target armv7a-linux-gnueabihf)

# Create an executable for each application
#file(GLOB apps *.bpf.c)
#foreach(app ${apps})
#    get_filename_component(app_stem ${app} NAME_WE)

    # Build object skeleton and depend skeleton on libbpf build
#    bpf_object(${app_stem} ${app_stem}.bpf.c)
#    add_dependencies(${app_stem}_skel libbpf-build)

#    add_executable(${app_stem} ${app_stem}.c src/bpf/program.bpf.h)
#    target_link_libraries(${app_stem} ${app_stem}_skel)
#endforeach()