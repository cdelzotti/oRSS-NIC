#ifndef __BPF_H
#define __BPF_H

#include "../env.h"

#define ETH_P_IP 0x0800

#define BPF_ANY 0

struct FiveTuple {
  uint32_t src_ip; /**4 bytes source IP address */
  uint32_t dst_ip; /** 4 bytes destination IP address */
  uint16_t src_port; /** 2 bytes source port */
  uint16_t dst_port; /** 2 bytes destination port */
  uint8_t proto;  /** 1 byte protocol */
  uint8_t pad[3]; /** 3 byte padding */
};

struct ConnectionState {
  uint8_t SYN;
  uint8_t SYNACK;
  uint8_t ACK;
  uint8_t FIN;
  uint8_t HANDLED;
  uint8_t pad[3];
};

enum TCP_FLAGS {
  ERROR = 0,
  NOT_TCP = 1,
  SYN = 2,
  SYNACK = 3,
  ACK = 4,
  FIN = 5,
  NO_FLAGS = 7,
  UDP = 8,
};

// For some reason, the kernel might not have the xdp_md struct defined
// in the vmlinux.h file. In this case, just define it here.
 struct xdp_md_copy {
 	uint32_t data;
 	uint32_t data_end;
 	uint32_t data_meta;
 	/* Below access go through struct xdp_rxq_info */
 	uint32_t ingress_ifindex; /* rxq->dev->ifindex */
 	uint32_t rx_queue_index;  /* rxq->queue_index  */
 };


#endif
