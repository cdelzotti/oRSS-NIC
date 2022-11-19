/**
 * @file openflow.h
 * @author Clément Delzotti (you@domain.com)
 * @brief oRSS specific OpenFlow functions
 * @version 0.1
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef OPENFLOW_H
#define OPENFLOW_H

#ifndef O_NONBLOCK
#define O_NONBLOCK	00004000
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openflow/openflow.h>
#include <fcntl.h>
#include "env.h"

/**
 * 64 bits implementation of the network <-> host byte order conversion
 */
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

// OpenFlow constants
#define OFP_VERSION 0x01
#define OFP_HEADER_LEN 8
#define OFP_MAX_PORT_NAME_LEN 16

// OpenFlow message types
#define OFP_HELLO 0x00
#define OFP_ECHO_REQUEST 0x02
#define OFP_ECHO_REPLY 0x03
#define OFP_FEATURES_REQUEST 0x05
#define OFP_FEATURES_REPLY 0x06
#define OFP_FLOW_MOD 0x08
#define OFP_STATS_REQUEST 0x10
#define OFP_STATS_REPLY 0x11


// OpenFlow stats request types
#define OFPST_FLOW 0x01

typedef int openflow_socket_fd;

/**
 * @brief Each OpenFlow message starts with a common 8 bytes header
 * 
 */
struct openflow_header {
    uint8_t version;
    uint8_t type;
    uint16_t length;
    uint32_t xid;
};

typedef struct openflow_header openflow_header;

/**
 * @brief OpenFlow FEATURES_REPLY message body without ports data
 * 
 */
struct openflow_features {
    uint64_t datapath_id;
    uint32_t n_buffers;
    uint8_t n_tables;
    uint8_t pad[3];
    uint32_t capabilities;
    uint32_t actions;
};

typedef struct openflow_features openflow_features;

/**
 * @brief OpenFlow FEATURES_REPLY ports data section
 * 
 */
struct openflow_port_data {
    uint16_t port_no;
    uint8_t hw_addr[6];
    char name[16];
    uint32_t config;
    uint32_t state;
    uint32_t curr;
    uint32_t advertised;
    uint32_t supported;
    uint32_t peer;
};

typedef struct openflow_port_data openflow_port_data;

typedef struct openflow_connection openflow_connection;

/**
 * @brief OpenFlow message structure, just a header and a body
 * 
 */
struct openflow_message {
    struct openflow_header header;
    void *data;
};

typedef struct openflow_message openflow_message;


struct openflow_flow_stats_request_header {
    uint16_t type;
    uint16_t flags;
};



typedef struct openflow_flow_stats_request_header openflow_flow_stats_request_header;

typedef openflow_flow_stats_request_header
    openflow_flow_stats_reply_header;

typedef struct ofp10_flow_stats_request openflow_flow_stats_request_body;

typedef struct ofp10_flow_stats openflow_flow_stats;

typedef struct ofp10_match openflow_match;

typedef struct openflow_action_header openflow_action_header;

enum OFPAT {
    OFPAT_OUTPUT = 0,
    OFPAT_SET_VLAN_VID = 1,
    OFPAT_SET_VLAN_PCP = 2,
    OFPAT_STRIP_VLAN = 3,
    OFPAT_SET_DL_SRC = 4,
    OFPAT_SET_DL_DST = 5,
    OFPAT_SET_NW_SRC = 6,
    OFPAT_SET_NW_DST = 7,
    OFPAT_SET_NW_TOS = 8,
    OFPAT_SET_TP_SRC = 9,
    OFPAT_SET_TP_DST = 10,
    OFPAT_ENQUEUE = 11,
    OFPAT_VENDOR = 0xffff
};

struct openflow_action_output {
    uint16_t type;
    uint16_t len;
    uint16_t port;
    uint16_t max_len;
};

typedef struct openflow_action_output openflow_action_output;

struct openflow_action_vlan_vid {
    uint16_t type;
    uint16_t len;
    uint16_t vlan_vid;
    uint8_t pad[2];
};

typedef struct openflow_action_vlan_vid openflow_action_vlan_vid;

struct action_descriptor {
    void *data;
};

typedef struct action_descriptor action_descriptor;


struct openflow_stats_request_message {
    openflow_header header;
    openflow_flow_stats_request_header request_header;
    openflow_flow_stats_request_body body;
};

typedef struct openflow_stats_request_message openflow_stats_request_message;

struct openflow_flows {
    uint16_t nb_flows;
    openflow_flow_stats flow_stats[MAX_HANDLED_FLOWS];
    uint8_t nb_actions[MAX_HANDLED_FLOWS];
    action_descriptor actions[MAX_HANDLED_FLOWS][MAX_ACTIONS];
};

typedef struct openflow_flows openflow_flows;


/**
 * @brief Abstraction of an OpenFlow connection, users just need to call the right functions to send and receive messages
 * 
 */
struct openflow_connection {
    openflow_socket_fd fd;
    openflow_socket_fd server_fd;
    struct sockaddr_in addr;
    struct openflow_features features;
    struct openflow_port_data *ports;
    uint8_t nb_ports;
    openflow_message msg_buffer[16];
    uint8_t nb_msg_buffered;
};

/**
 * @brief Initialize an OpenFlow connection. The connection is established over TCP on the port specified in the env.h file.
 * 
 * @param conn : the connection to initialize
 */
void openflow_create_connection(openflow_connection *conn);

/**
 * @brief Terminates the connection and frees the resources
 * 
 * @param conn : the connection to terminate
 */
void openflow_terminate_connection(openflow_connection *conn);

/**
 * @brief Handle OpenFlow controlling such as ECHO_REQUEST, it will empty both connection buffers (Messages buffered in openflow_connection::msg_buffer and the socket buffer). It is user responsibility to call this function periodically to avoid connection timeout.
 * 
 * @param conn : the connection to use
 */
void openflow_control(openflow_connection *conn);

/**
 * @brief Get a details of flows
 * 
 * @param conn : the connection to use
 * @param flows : flow structure that will be filled
 */
void openflow_get_flows(openflow_connection *connection, openflow_flows *flows);

/**
 * @brief Free the flow structure
 * 
 * @param flows : the flow structure to free
 */
void openflow_free_flows(openflow_flows *flows);

#endif