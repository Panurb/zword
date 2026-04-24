#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "network.h"

#include "game.h"

Network network;


static bool winsock_initialized = false;


bool network_init() {
    memset(&network, 0, sizeof(Network));
    network.mode = NET_MODE_NONE;
    network.sock = NET_INVALID_SOCKET;
    network.local_player_slot = 0;
    network.num_clients = 0;
    network.tick = 0;
    network.game_started = false;
    network.own_ip[0] = '\0';
    network.host_ip[0] = '\0';

    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        network.clients[i].connected = false;
        network.clients[i].player_slot = -1;
        network.clients[i].last_recv_time = 0.0f;
        network.clients[i].ip[0] = '\0';
        network.clients[i].player_name[0] = '\0';
    }

    #ifdef _WIN32
        if (!winsock_initialized) {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                LOG_ERROR("WSAStartup failed: %d", WSAGetLastError());
                return false;
            }
            winsock_initialized = true;
        }
    #endif

    return true;
}


void network_shutdown() {
    if (network.sock != NET_INVALID_SOCKET) {
        #ifdef _WIN32
            closesocket(network.sock);
        #else
            close(network.sock);
        #endif
        network.sock = NET_INVALID_SOCKET;
    }

    #ifdef _WIN32
        if (winsock_initialized) {
            WSACleanup();
            winsock_initialized = false;
        }
    #endif

    network.mode = NET_MODE_NONE;
}


static bool create_udp_socket() {
    network.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (network.sock == NET_INVALID_SOCKET) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    // Set non-blocking
    #ifdef _WIN32
        u_long nonblocking = 1;
        ioctlsocket(network.sock, FIONBIO, &nonblocking);
    #else
        int flags = fcntl(network.sock, F_GETFL, 0);
        fcntl(network.sock, F_SETFL, flags | O_NONBLOCK);
    #endif

    return true;
}


void get_own_ip(char ip_str[INET_ADDRSTRLEN]) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        return;
    }

    struct addrinfo hints, *info, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, NULL, &hints, &info) != 0) {
        perror("getaddrinfo");
        return;
    }

    for (p = info; p != NULL; p = p->ai_next) {
        struct sockaddr_in *addr = (struct sockaddr_in *)p->ai_addr;

        if (addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) {
            continue;  // Skip loopback address
        }

        inet_ntop(AF_INET, &addr->sin_addr, ip_str, INET_ADDRSTRLEN);
        break;
    }

    freeaddrinfo(info);
}


bool network_host_start(int port) {
    if (!create_udp_socket()) return false;

    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = htons((unsigned short)port);

    if (bind(network.sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) != 0) {
        LOG_ERROR("Failed to bind socket on port %d", port);
        network_shutdown();
        return false;
    }

    network.mode = NET_MODE_HOST;
    network.local_player_slot = 0;  // Host is always player 0

    get_own_ip(network.own_ip);
    LOG_INFO("Hosting on %s:%d", network.own_ip, port);

    return true;
}


bool network_client_connect(const char* host_ip, int port) {
    if (!create_udp_socket()) return false;

    // Bind to any port for receiving
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = INADDR_ANY;
    bind_addr.sin_port = 0;  // OS picks a port

    if (bind(network.sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr)) != 0) {
        LOG_ERROR("Failed to bind client socket");
        network_shutdown();
        return false;
    }

    // Store host address
    memset(&network.host_addr, 0, sizeof(network.host_addr));
    network.host_addr.sin_family = AF_INET;
    network.host_addr.sin_port = htons((unsigned short)port);
    inet_pton(AF_INET, host_ip, &network.host_addr.sin_addr);

    network.mode = NET_MODE_CLIENT;

    // Send JOIN packet
    JoinPacket join;
    join.header.type = PACKET_JOIN;
    join.header.tick = 0;
    join.header.size = sizeof(PacketHeader);
    strcpy(join.player_name, game_data->player_name);
    network_send_to_host(&join, sizeof(join));

    LOG_INFO("Connecting to %s:%d", host_ip, port);
    return true;
}


bool network_send_to(struct sockaddr_in* addr, const void* data, int size) {
    int sent = sendto(network.sock, (const char*)data, size, 0,
                      (struct sockaddr*)addr, sizeof(struct sockaddr_in));
    return sent == size;
}


void network_broadcast(const void* data, int size) {
    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        if (network.clients[i].connected) {
            network_send_to(&network.clients[i].addr, data, size);
        }
    }
}


bool network_send_to_host(const void* data, int size) {
    return network_send_to(&network.host_addr, data, size);
}


int network_receive(void* buf, int buf_size, struct sockaddr_in* from_addr) {
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);

    int received = recvfrom(network.sock, (char*)buf, buf_size, 0,
                            (struct sockaddr*)&addr, &addr_len);

    if (received > 0 && from_addr) {
        *from_addr = addr;
    }

#ifdef _WIN32
    if (received < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
        return 0;
    }
#else
    if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;
    }
#endif

    return received;
}


static bool addr_equals(struct sockaddr_in* a, struct sockaddr_in* b) {
    return a->sin_addr.s_addr == b->sin_addr.s_addr && a->sin_port == b->sin_port;
}


void network_host_accept_clients() {
    struct sockaddr_in from_addr;

    while (true) {
        int received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from_addr);
        if (received <= 0) break;
        if (received < (int)sizeof(PacketHeader)) continue;

        PacketHeader* header = (PacketHeader*)network.recv_buf;

        if (header->type == PACKET_JOIN) {
            // Check if already connected
            int existing = -1;
            for (int i = 0; i < NET_MAX_CLIENTS; i++) {
                if (network.clients[i].connected && addr_equals(&network.clients[i].addr, &from_addr)) {
                    existing = i;
                    break;
                }
            }

            if (existing >= 0) {
                // Re-send ack
                JoinAckPacket ack;
                ack.header.type = PACKET_JOIN_ACK;
                ack.header.tick = 0;
                ack.header.size = sizeof(JoinAckPacket);
                ack.player_slot = network.clients[existing].player_slot;
                network_send_to(&from_addr, &ack, sizeof(ack));
                continue;
            }

            // Find free slot
            if (network.num_clients >= NET_MAX_CLIENTS) {
                LOG_WARNING("Max clients reached, rejecting connection");
                continue;
            }

            int slot = -1;
            for (int i = 0; i < NET_MAX_CLIENTS; i++) {
                if (!network.clients[i].connected) {
                    slot = i;
                    break;
                }
            }

            if (slot >= 0) {
                JoinPacket* join_pkt = (JoinPacket*)network.recv_buf;
                network.clients[slot].connected = true;
                network.clients[slot].addr = from_addr;
                network.clients[slot].player_slot = slot + 1;  // Host is slot 0, clients are 1-3
                network.clients[slot].last_recv_time = 0.0f;  // Will be set on first input packet
                strcpy(network.clients[slot].player_name, join_pkt->player_name);
                strcpy(network.clients[slot].ip, inet_ntoa(from_addr.sin_addr));
                network.num_clients++;

                JoinAckPacket ack;
                ack.header.type = PACKET_JOIN_ACK;
                ack.header.tick = 0;
                ack.header.size = sizeof(JoinAckPacket);
                ack.player_slot = network.clients[slot].player_slot;
                network_send_to(&from_addr, &ack, sizeof(ack));

                LOG_INFO("Client connected as player %d, name %s", ack.player_slot, join_pkt->player_name);
            }
        }
    }
}


int network_check_timeouts(float current_time) {
    int disconnected_mask = 0;

    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        if (!network.clients[i].connected) continue;
        // Skip timeout check until first packet received (last_recv_time == 0)
        if (network.clients[i].last_recv_time == 0.0f) continue;

        float elapsed = current_time - network.clients[i].last_recv_time;
        if (elapsed > NET_DISCONNECT_TIMEOUT) {
            int slot = network.clients[i].player_slot;
            LOG_WARNING("Client player %d timed out (%.1fs)", slot, elapsed);
            network.clients[i].connected = false;
            network.clients[i].player_slot = -1;
            network.clients[i].last_recv_time = 0.0f;
            network.num_clients--;
            disconnected_mask |= (1 << slot);
        }
    }

    return disconnected_mask;
}
