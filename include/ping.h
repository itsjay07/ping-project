#ifndef PING_H
#define PING_H

#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

// Default values
#define DEFAULT_PACKET_SIZE 56
#define DEFAULT_INTERVAL 1
#define DEFAULT_TIMEOUT 10
#define DEFAULT_COUNT 4
#define ICMP_HEADER_SIZE 8

// Structure to hold ping statistics
typedef struct {
    int sent;
    int received;
    double min_rtt;
    double max_rtt;
    double total_rtt;
    struct timeval start_time;
    struct timeval end_time;
} ping_stats_t;

// Structure to hold program options
typedef struct {
    int count;              // -c option
    int packet_size;        // -s option
    double interval;        // -i option
    int ttl;                // -t option
    int timeout;            // -W option
    int deadline;           // -w option
    int quiet;              // -q option
    char source_ip[64];      // -I option
    char payload_pattern[256]; // -p option
    char host[256];          // target host
} ping_options_t;

// Function prototypes
unsigned short calculate_checksum(void *b, int len);
int create_raw_socket();
int resolve_host(const char *hostname, struct sockaddr_in *addr);
void create_icmp_packet(char *packet, int seq, int size, ping_options_t *opts);
void print_usage(char *progname);
int parse_args(int argc, char *argv[], ping_options_t *opts);
void print_stats(ping_stats_t *stats, struct sockaddr_in *dest);
void handle_signal(int sig);

#endif
