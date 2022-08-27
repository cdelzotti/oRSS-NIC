#ifndef __BPF_H
#define __BPF_H

#include "../env.h"

struct FiveTuple {
  uint32_t src_ip;
  uint32_t dst_ip;
  uint16_t src_port;
  uint16_t dst_port;
  uint8_t proto;
};

struct ConnectionState {
    uint32_t packets;
    uint64_t bytes;
};

#endif