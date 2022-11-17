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



#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

/**
 * @brief Abstraction of an OpenFlow connection, users just need to call the right functions to send and receive messages
 * 
 */
struct openflow_connection {
    openflow_socket_fd fd;
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

#endif