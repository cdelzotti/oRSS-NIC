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
    int socket_fd, valread;
    int opt = 1;
    int addrlen = sizeof(conn->addr);

    // Get socket FD
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Could not get socket FD\n");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 6666
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                                                  &opt, sizeof(opt)))
    {
        printf("Could not set socket options\n");
        exit(EXIT_FAILURE);
    }
    conn->addr.sin_family = AF_INET;
    conn->addr.sin_addr.s_addr = INADDR_ANY;
    conn->addr.sin_port = htons( OF_PORT );

    // Attach socket to port
    if (bind(socket_fd, (struct sockaddr*)&conn->addr,
                                 sizeof(conn->addr))<0) {
        printf("Could not bind socket to port");
        exit(EXIT_FAILURE);
    }
    if (listen(socket_fd, 1) < 0) {
        printf("Could not listen on socket");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client to connect...\n");
    conn->fd = accept(socket_fd, (struct sockaddr*)&conn->addr,(socklen_t*)&addrlen);
    if (conn-> fd < 0) {
        printf("Error accepting connection: %s\n", strerror(conn->fd));
        exit(EXIT_FAILURE);
    }
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
    if (valread < 0) {
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
    openflow_message message;
    // Empty the connection buffer
    for (uint8_t i = 0; i < connection->nb_msg_buffered; i++) {
        control_logic(connection, &message);
        free_openflow_message_body(&message);
    }
    connection->nb_msg_buffered = 0;
    // Empty the socket buffer
    while (read_openflow_message(connection->fd, &message) > 0) {
        control_logic(connection, &message);
        free_openflow_message_body(&message);
    }
}

void openflow_terminate_connection(openflow_connection *connection){
    // Close connection
    close(connection->fd);
    shutdown(connection->fd, SHUT_RDWR);
    // Free ports
    free(connection->ports);
}


