// Indexes of the interfaces to attach the XDP programs to
#define RX_IFINDEX 4
#define TX_IFINDEX 3

// OVS info
#define OVS_BRIDGE "ovsbr1"
// Index of the interface in the OVS bridge, run `ovs-ofctl show <bridge-name>` to find out
#define RX_SWITCH_IFINDEX 4
#define TX_SWITCH_IFINDEX 1

// Size of the ringbuffer
#define RING_SIZE 16
// Size of the hashmap
#define HASHMAP_SIZE 1024

// In a situation of OVS Hardware offloading, you might want to redirect the traffic directly
// to the opposite port, bypassing OVS detection and connection initialization. 
#define XDP_FORWARDING 0

// XDP loading mode, choose SKB mode if driver does not support native XDP
#define XDP_LOADING_MODE (1U << 1) // XDP_FLAGS_SKB_MODE
// #define XDP_LOADING_MODE (1U << 2) // XDP_FLAGS_DRV_MODE