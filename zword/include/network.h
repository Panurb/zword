#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "util.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET NetSocket;
    #define NET_INVALID_SOCKET INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    typedef int NetSocket;
    #define NET_INVALID_SOCKET -1
#endif

#define NET_DEFAULT_PORT 12345
#define NET_MAX_PACKET_SIZE 65000
#define NET_MAX_CLIENTS 3
#define NET_DISCONNECT_TIMEOUT 2.0f  // seconds without receiving a packet before disconnect

typedef enum {
    NET_MODE_NONE,
    NET_MODE_HOST,
    NET_MODE_CLIENT
} NetMode;

typedef enum {
    PACKET_JOIN,
    PACKET_JOIN_ACK,
    PACKET_KEEPALIVE,
    PACKET_INPUT,
    PACKET_SNAPSHOT,
    PACKET_START_GAME,
    PACKET_END_GAME,
    PACKET_LOBBY_INFO
} PacketType;

typedef enum {
    MATCH_END_GAME_OVER,
    MATCH_END_WIN
} MatchEndType;

#pragma pack(push, 1)

typedef struct {
    uint8_t type;
    uint32_t tick;
    uint16_t size;
} PacketHeader;

typedef struct {
    PacketHeader header;
    char player_name[32];
} JoinPacket;

typedef struct {
    PacketHeader header;
    uint8_t player_slot;
} JoinAckPacket;

typedef struct {
    PacketHeader header;
} KeepAlivePacket;

typedef struct {
    PacketHeader header;
    char map_name[128];
    uint8_t game_mode;
    uint8_t num_players;
} StartGamePacket;

typedef struct {
    PacketHeader header;
    uint8_t end_type;
} EndGamePacket;

typedef struct {
    PacketHeader header;
    char map_name[128];
    uint8_t game_mode;
    uint8_t num_players;
    char player_names[NET_MAX_CLIENTS + 1][32];
    uint8_t point_limit;
} LobbyInfoPacket;

#pragma pack(pop)

typedef struct {
    bool connected;
    struct sockaddr_in addr;
    String ip;
    int player_slot;  // 0-3
    float last_recv_time;  // SDL_GetTicks() / 1000.0f when last packet received
    String player_name;
} ClientInfo;

typedef struct {
    NetMode mode;
    NetSocket sock;
    struct sockaddr_in host_addr;
    int local_player_slot;
    int num_clients;
    ClientInfo clients[NET_MAX_CLIENTS];
    uint32_t tick;
    uint8_t recv_buf[NET_MAX_PACKET_SIZE];
    uint8_t send_buf[NET_MAX_PACKET_SIZE];
    bool game_started;
    char own_ip[INET_ADDRSTRLEN];
    char host_ip[INET_ADDRSTRLEN];
} Network;

extern Network network;

bool network_init();
void network_shutdown();

bool network_host_start(int port);
bool network_client_connect(const char* host_ip, int port);

// Send raw data to a specific address
bool network_send_to(struct sockaddr_in* addr, const void* data, int size);

// Send to all connected clients (host only)
void network_broadcast(const void* data, int size);

// Send to host (client only)
bool network_send_to_host(const void* data, int size);

// Non-blocking receive. Returns bytes received, 0 if nothing, -1 on error.
int network_receive(void* buf, int buf_size, struct sockaddr_in* from_addr);

// Host: check for new client connections (JOIN packets)
void network_host_accept_clients();

// Host: check for timed-out clients and disconnect them.
// Returns bitmask of disconnected player slots (bit N = slot N disconnected).
int network_check_timeouts(float current_time);
