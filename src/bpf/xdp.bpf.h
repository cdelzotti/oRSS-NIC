#ifndef __BPF_H
#define __BPF_H

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

// For some reason, the kernel might not have the xdp_md struct defined
// in the vmlinux.h file. In this case, just define it here.
// struct xdp_md {
// 	__u32 data;
// 	__u32 data_end;
// 	__u32 data_meta;
// 	/* Below access go through struct xdp_rxq_info */
// 	__u32 ingress_ifindex; /* rxq->dev->ifindex */
// 	__u32 rx_queue_index;  /* rxq->queue_index  */
// };


#endif