#include "openflow.h"

uint32_t transaction_id = 0;

/**
 * @brief Waits for a connection to be established, then returns the socket file descriptor
 * and the address of the client.
 * 
 * @param conn : the connection to initialize
 * @return void : Information about the connection are stored in the conn structure, and the program exits if an error occurs
 */
void get_socket(openflow_connection *conn) {
    int valread;
    int opt = 1;
    int addrlen = sizeof(conn->addr);

    // Get socket FD
    if ((conn->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Could not get socket FD\n");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(conn->server_fd, SOL_SOCKET, SO_REUSEADDR,
                                                  &opt, sizeof(opt)))
    {
        printf("Could not set socket options\n");
        exit(EXIT_FAILURE);
    }
    conn->addr.sin_family = AF_INET;
    conn->addr.sin_addr.s_addr = INADDR_ANY;
    conn->addr.sin_port = htons( OF_PORT );

    // Attach socket to port
    if (bind(conn->server_fd, (struct sockaddr*)&conn->addr,
                                 sizeof(conn->addr))<0) {
        printf("Could not bind socket to port");
        exit(EXIT_FAILURE);
    }
    if (listen(conn->server_fd, 1) < 0) {
        printf("Could not listen on socket");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client to connect...\n");
    conn->fd = accept4(conn->server_fd, (struct sockaddr*)&conn->addr,
                       (socklen_t*)&addrlen, SOCK_NONBLOCK);
    // conn->fd = accept(conn->server_fd, (struct sockaddr*)&conn->addr,(socklen_t*)&addrlen);
    if (conn-> fd < 0) {
        printf("Error accepting connection: %s\n", strerror(conn->fd));
        exit(EXIT_FAILURE);
    }
    // Set socket to non-blocking
    // int flags = fcntl(conn->fd, F_GETFL, 0);
    // if (flags == -1) {
    //     printf("Could not configure socket to non-blocking\n");
    //     exit(EXIT_FAILURE);
    // };
    // flags = 0 ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    // fcntl(conn->fd, F_SETFL, fcntl(conn->fd, F_GETFL, 0) | O_NONBLOCK);
    printf("Client connected!\n");
}

/**
 * @brief Reads a message from the connection and stores it in the message structure
 * 
 * @param socket_fd : the socket file descriptor
 * @param message : the message structure to fill
 * @return int : -1 if an error occured, 0 if nothing was read, 1 if a message was read
 */
int read_openflow_message(openflow_socket_fd socket_fd,struct openflow_message *message){
    // Read header information
    int valread = read(socket_fd, &message->header, OFP_HEADER_LEN);
    if (valread < -1) {
        printf("Error reading headers from socket: %s\n", strerror(valread));
        return -1;
    }
    message->header.length = ntohs(message->header.length);
    message->header.xid = ntohl(message->header.xid);
    // If we read a message,
    if (valread > 0) {
        // Read message body if there is one
        if (message->header.length > OFP_HEADER_LEN) {
            message->data = malloc(message->header.length - OFP_HEADER_LEN);
            valread = read(socket_fd, message->data, message->header.length - OFP_HEADER_LEN);
            if (valread < 0) {
                printf("Error reading body from socket: %s\n", strerror(valread));
            }
        }
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Sends a message to the connection
 * 
 * @param socket_fd : the socket file descriptor
 * @param message : the message structure to send
 * @return int : -1 if an error occured, 0 if nothing was sent, 1 if a message was sent
 */
void send_openflow_hello(openflow_socket_fd socket_fd){
    struct openflow_header header;
    header.version = OFP_VERSION;
    header.type = OFP_HELLO;
    header.length = htons(OFP_HEADER_LEN);
    header.xid = htonl(transaction_id++);
    int valwrite = write(socket_fd, &header, OFP_HEADER_LEN);
    if (valwrite < 0) {
        printf("Error writing HELLO message to socket: %s\n", strerror(valwrite));
    }
}

/**
 * @brief Sends a features request to the switch
 * 
 * @param socket_fd : the socket file descriptor
 * @param message : the message structure to send
 */
uint32_t send_openflow_features_request(openflow_socket_fd socket_fd){
    struct openflow_header header;
    header.version = OFP_VERSION;
    header.type = OFP_FEATURES_REQUEST;
    header.length = htons(OFP_HEADER_LEN);
    header.xid = htonl(transaction_id);
    transaction_id++;
    int valwrite = write(socket_fd, &header, OFP_HEADER_LEN);
    if (valwrite < 0) {
        printf("Error writing FEATURES_REQUEST message to socket: %s\n", strerror(valwrite));
    }
    return transaction_id - 1;
}

/**
 * @brief Sends a echo_reply message to the switch
 * 
 * @param socket_fd : the socket file descriptor
 */
void send_openflow_echo_reply(openflow_socket_fd socket_fd){
    struct openflow_header header;
    header.version = OFP_VERSION;
    header.type = OFP_ECHO_REPLY;
    header.length = htons(OFP_HEADER_LEN);
    header.xid = 0;
    int valwrite = write(socket_fd, &header, OFP_HEADER_LEN);
    if (valwrite < 0) {
        printf("Error writing ECHO_REPLY message to socket: %s\n", strerror(valwrite));
    }
}

void free_openflow_message_body(struct openflow_message *message){
    if (message->header.length > OFP_HEADER_LEN) {
        free(message->data);
    }
}


/**
 * @brief Wait for a particular message type from the switch
 * 
 * @param conn : the connection to read from
 * @param type : the type of message to wait for
 * @param msg : the message structure to fill
 * @param xid : the transaction ID to wait for (0 to ignore)
 */
void openflow_wait_for_message(openflow_connection *conn, uint8_t type, openflow_message *msg, uint32_t xid) {
    uint8_t msg_received = 0;
    while (!msg_received) {
        if (read_openflow_message(conn->fd, msg) > 0) {
            if (msg->header.type == type && (xid == 0 || msg->header.xid == xid)) {
                msg_received = 1;
            } else {
                // We received a message that is not the one we are waiting for
                // Let's buffer it for the openflow_control function
                conn->msg_buffer[conn->nb_msg_buffered] = *msg;
                conn->nb_msg_buffered++;
            }
        }
    }
}

/**
 * @brief Parses the features reply message and stores it in the openflow_connection structure
 * 
 * @param msg : the message structure to parse
 * @param conn : the connection structure
 */
void parse_features_reply(openflow_message *msg, openflow_connection *conn) {
    openflow_features *switch_features = (struct openflow_features *)msg->data;
    // Parse features
    conn->features.datapath_id = ntohll(switch_features->datapath_id);
    conn->features.n_buffers = ntohl(switch_features->n_buffers);
    conn->features.n_tables = switch_features->n_tables;
    conn->features.capabilities = ntohl(switch_features->capabilities);
    conn->features.actions = ntohl(switch_features->actions);
    // Parse ports
    if (msg->header.length - OFP_HEADER_LEN - sizeof(openflow_features) > 0) {
        openflow_port_data *port_data = (openflow_port_data*)(msg->data + sizeof(openflow_features));
        conn->nb_ports = (msg->header.length - OFP_HEADER_LEN - sizeof(openflow_features)) / sizeof(openflow_port_data);
        conn->ports = malloc(conn->nb_ports * sizeof(openflow_port_data));
        for (int i = 0; i < conn->nb_ports; i++) {
            conn->ports[i].port_no = ntohs(port_data[i].port_no);
            for (uint8_t mac_byte = 0; mac_byte < 6; mac_byte++)
            {
                conn->ports[i].hw_addr[mac_byte] = port_data[i].hw_addr[mac_byte];
            }
            for (uint8_t name_byte = 0; name_byte < OFP_MAX_PORT_NAME_LEN; name_byte++)
            {
                conn->ports[i].name[name_byte] = port_data[i].name[name_byte];
            }
            conn->ports[i].config = ntohl(port_data[i].config);
            conn->ports[i].state = ntohl(port_data[i].state);
            conn->ports[i].curr = ntohl(port_data[i].curr);
            conn->ports[i].advertised = ntohl(port_data[i].advertised);
            conn->ports[i].supported = ntohl(port_data[i].supported);
            conn->ports[i].peer = ntohl(port_data[i].peer);
        }
    } else {
        conn->nb_ports = 0;
    }
}

void openflow_create_connection(openflow_connection *connection){
    get_socket(connection);
    transaction_id = rand();
    openflow_message message;
    openflow_wait_for_message(connection, OFP_HELLO, &message,0);
    send_openflow_hello(connection->fd);
    uint32_t features_xid = send_openflow_features_request(connection->fd);
    openflow_wait_for_message(connection, OFP_FEATURES_REPLY, &message, features_xid);
    parse_features_reply(&message, connection);
    free_openflow_message_body(&message);
    printf("Connected to switch %lx\n", connection->features.datapath_id);
}

void ntoh_openflow_match(openflow_match *match){
    match->wildcards = ntohl(match->wildcards);
    match->in_port = ntohs(match->in_port);
    match->dl_vlan = ntohs(match->dl_vlan);
    match->dl_type = ntohs(match->dl_type);
    match->nw_src = ntohl(match->nw_src);
    match->nw_dst = ntohl(match->nw_dst);
    match->tp_src = ntohs(match->tp_src);
    match->tp_dst = ntohs(match->tp_dst);
}

void ntoh_openflow_flow_stats(openflow_flow_stats *flow_stats){
    flow_stats->length = ntohs(flow_stats->length);
    flow_stats->duration_sec = ntohs(flow_stats->duration_sec);
    flow_stats->duration_nsec = ntohs(flow_stats->duration_nsec);
    flow_stats->priority = ntohs(flow_stats->priority);
    flow_stats->idle_timeout = ntohs(flow_stats->idle_timeout);
    flow_stats->hard_timeout = ntohs(flow_stats->hard_timeout);
    flow_stats->cookie.hi = ntohl(flow_stats->cookie.hi);
    flow_stats->cookie.lo = ntohl(flow_stats->cookie.lo);
    flow_stats->packet_count.lo = ntohl(flow_stats->packet_count.lo);
    flow_stats->packet_count.hi = ntohl(flow_stats->packet_count.hi);
    flow_stats->byte_count.lo = ntohl(flow_stats->byte_count.lo);
    flow_stats->byte_count.hi = ntohl(flow_stats->byte_count.hi);
    ntoh_openflow_match(&flow_stats->match);
}

void ntoh_openflow_action_output(openflow_action_output *action_output){
    action_output->port = ntohs(action_output->port);
    action_output->max_len = ntohs(action_output->max_len);
    action_output->type = ntohs(action_output->type);
    action_output->len = ntohs(action_output->len);
}

void ntoh_openflow_action_vlan_vid(openflow_action_vlan_vid *action_vlan_vid){
    action_vlan_vid->vlan_vid = ntohs(action_vlan_vid->vlan_vid);
    action_vlan_vid->type = ntohs(action_vlan_vid->type);
    action_vlan_vid->len = ntohs(action_vlan_vid->len);
}


void openflow_get_flows(openflow_connection *connection, openflow_flows *flows){
    // Create a flow request
    int current_xid = transaction_id;
    transaction_id++;
    openflow_stats_request_message flow_request = {0};
    flow_request.header.version = OFP_VERSION;
    flow_request.header.type = OFP_STATS_REQUEST;
    flow_request.header.length = htons(sizeof(openflow_stats_request_message));
    flow_request.header.xid = htonl(current_xid);
    flow_request.request_header.type = htons(OFPST_FLOW);
    flow_request.request_header.flags = 0;
    // Set wildcard to match all flows
    flow_request.body.match.wildcards = htonl(OFPFW10_ALL);
    flow_request.body.table_id = 0xff;
    flow_request.body.out_port = OFPP_NONE;

    // Send the flow request
    int valwrite = write(connection->fd, &flow_request, sizeof(openflow_stats_request_message));
    if (valwrite < 0) {
        printf("Error writing FLOW_REQUEST message to socket: %s\n", strerror(valwrite));
        return;
    }
    // Wait for the reply
    openflow_message messages[MAX_STATS_REPLY];
    int nb_messages = 0;
    uint8_t reply_fully_received = 0;
    while (!reply_fully_received){
        openflow_wait_for_message(connection, OFP_STATS_REPLY, &messages[nb_messages], current_xid);
        nb_messages++;
        openflow_flow_stats_reply_header *reply_header = (openflow_flow_stats_reply_header *)messages[nb_messages-1].data;
        if ((ntohs(reply_header->flags) & OFPSF_REPLY_MORE) == 0){
            reply_fully_received = 1;
        } else {
        }
    }
    // Parse the replies
    flows->nb_flows = 0;
    for (uint8_t i = 0; i < nb_messages; i++){
        void *response_end = messages[i].data + messages[i].header.length - OFP_HEADER_LEN;
        // Place the pointer at the beginning of the first item
        void *response_head = messages[i].data + sizeof(openflow_flow_stats_reply_header);
        // While we have not reached the end of the message
        while (response_head < response_end){
            // Parse the flow statistics
            flows->flow_stats[flows->nb_flows] = *(openflow_flow_stats *)response_head;
            ntoh_openflow_flow_stats(&flows->flow_stats[flows->nb_flows]);
            // Go to action details
            void *actions_end = response_head + flows->flow_stats[flows->nb_flows].length;
            response_head += sizeof(openflow_flow_stats);
            // Parse the actions details
            uint8_t *nb_actions = &flows->nb_actions[flows->nb_flows];
            while (response_head < actions_end){
                uint16_t action_type = ntohl(*(uint16_t *)response_head);
                // Read the action
                switch (action_type)
                {
                case OFPAT_OUTPUT:
                    flows->actions[flows->nb_flows][*nb_actions].data = malloc(sizeof(openflow_action_output));
                    *(openflow_action_output *)flows->actions[flows->nb_flows][*nb_actions].data = *(openflow_action_output *)response_head;
                    ntoh_openflow_action_output((openflow_action_output *)flows->actions[flows->nb_flows][*nb_actions].data);
                    response_head += sizeof(openflow_action_output);
                    break;
                case OFPAT_SET_VLAN_VID:
                    flows->actions[flows->nb_flows][*nb_actions].data = malloc(sizeof(openflow_action_vlan_vid));
                    *(openflow_action_vlan_vid *)flows->actions[flows->nb_flows][*nb_actions].data = *(openflow_action_vlan_vid *)response_head;
                    ntoh_openflow_action_vlan_vid((openflow_action_vlan_vid *)flows->actions[flows->nb_flows][*nb_actions].data);
                    response_head += sizeof(openflow_action_vlan_vid);
                    break;
                default:
                    response_head = actions_end;
                    break;
                }
                (*nb_actions)++;
            }
            response_head = actions_end;
            flows->nb_flows++;
        }
    }
}


void openflow_free_flows(openflow_flows *flows){
    for (uint8_t i = 0; i < flows->nb_flows; i++){
        for (uint8_t j = 0; j < flows->nb_actions[i]; j++){
            free(flows->actions[i][j].data);
        }
    }
}

/**
 * @brief Individual logic for each OpenFlow message type
 * 
 * @param connection 
 * @param message 
 */
void control_logic(openflow_connection *connection, openflow_message *message){
    switch (message->header.type) {
        case OFP_ECHO_REQUEST:
            printf("Received PING\n");
            send_openflow_echo_reply(connection->fd);
            printf("Sent PONG\n");
            break;
        default:
            break;
    }
}

void openflow_control(openflow_connection *connection){
    openflow_message message = {0};
    // Empty the connection buffer
    for (uint8_t i = 0; i < connection->nb_msg_buffered; i++) {
        control_logic(connection, &message);
        free_openflow_message_body(&message);
    }
    connection->nb_msg_buffered = 0;
    // printf("Emptied connection buffer\n");
    // Empty the socket buffer
    // printf("Reading from socket\n");
    while (read_openflow_message(connection->fd, &message) > 0) {
        printf("[CONTROL] Received message of type %u\n", message.header.type);
        control_logic(connection, &message);
        free_openflow_message_body(&message);
    }
    // printf("Emptied socket buffer\n");
}

void openflow_terminate_connection(openflow_connection *connection){
    // Close connection socket
    close(connection->fd);
    shutdown(connection->fd, SHUT_RDWR);
    // Close server socket
    close(connection->server_fd);
    // Free ports
    free(connection->ports);
}


