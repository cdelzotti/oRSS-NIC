// OVS info
#define OVS_HOST_IFINDEX 2
#define OVS_NETWORK_IFINDEX 1
#define OVS_OUTPUT_ACTION_MAX_LEN 65535

// Size of the ringbuffer
#define RING_SIZE 16
// Size of the hashmap
#define HASHMAP_SIZE 1024

// Number of seconds before calling a connection timeout
#define CONN_TIMEOUT 15

// Number of cores to use
#define NB_CORES 8
// The imbalance threshold to trigger a rebalancing
#define IMBALANCE_THRESHOLD 0.1
// The maximum number of iterations to try to rebalance the flows
#define MAX_REBALANCE_ITERATIONS 10

// The listening OpenFlow port
#define OF_PORT 6666
// The maximum number of handled flows
#define MAX_HANDLED_FLOWS 1024
// The maximum number of consecutive OpenFlow STATS_REPLY messages
#define MAX_STATS_REPLY 32
// The maximum number of actions in a flow
#define MAX_ACTIONS 4


// In a situation of OVS Hardware offloading, you might want to redirect the traffic directly
// to the opposite port, bypassing OVS detection and connection initialization.
#define XDP_FORWARDING 0

// XDP loading mode, choose SKB mode if driver does not support native XDP
#define XDP_LOADING_MODE (1U << 1) // XDP_FLAGS_SKB_MODE
// #define XDP_LOADING_MODE (1U << 2) // XDP_FLAGS_DRV_MODE
