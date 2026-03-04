#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include "../include/ping.h"

// Global variables
ping_stats_t stats = {0, 0, 999999, 0, 0};
int running = 1;

// Signal Handler (for Ctrl+C)
void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

// Calculate ICMP Checksum
unsigned short calculate_checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Create Raw Socket
int create_raw_socket() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        perror("socket");
        fprintf(stderr, "Need root privileges! Try: sudo\n");
    }
    return sock;
}

// Resolve Hostname to IP
int resolve_host(const char *hostname, struct sockaddr_in *addr) {
    struct hostent *host = gethostbyname(hostname);
    if (!host) {
        fprintf(stderr, "ping: unknown host %s\n", hostname);
        return -1;
    }
    
    addr->sin_family = AF_INET;
    memcpy(&addr->sin_addr, host->h_addr_list[0], host->h_length);
    return 0;
}

// Create ICMP Packet
void create_icmp_packet(char *packet, int seq, int size, ping_options_t *opts) {
    struct icmp *icmp_hdr = (struct icmp *)packet;
    
    icmp_hdr->icmp_type = ICMP_ECHO;
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_id = getpid() & 0xFFFF;
    icmp_hdr->icmp_seq = seq;
    
    struct timeval *tv = (struct timeval *)(packet + ICMP_HEADER_SIZE);
    gettimeofday(tv, NULL);
    
    char *payload = packet + ICMP_HEADER_SIZE + sizeof(struct timeval);
    int payload_len = size - ICMP_HEADER_SIZE - sizeof(struct timeval);
    
    if (strlen(opts->payload_pattern) > 0) 
    {
        int pattern_len = strlen(opts->payload_pattern);
        for (int i = 0; i < payload_len; i++) {
            payload[i] = opts->payload_pattern[i % pattern_len];
        }
    } 
    
    else 
    {
        memset(payload, 0, payload_len);
    }
    
    icmp_hdr->icmp_cksum = calculate_checksum(packet, size);
}

// Print Usage/Help
void print_usage(char *progname) {
    printf("Usage: %s [OPTIONS] HOST\n", progname);
    printf("Send ICMP ECHO_REQUESTs to HOST\n");
    printf("\n");
    printf("-c CNT          Send only CNT pings (default: 4)\n");
    printf("-s SIZE         Send SIZE data bytes in packets (default 56)\n");
    printf("-i SECS         Interval between pings (default 1)\n");
    printf("-t TTL          Set Time To Live\n");
    printf("-W SEC          Seconds to wait for response (default 10)\n");
    printf("-w SEC          Seconds until ping exits\n");
    printf("-q              Quiet, only display output at start/finish\n");
}

// Parse Command Line Arguments
int parse_args(int argc, char *argv[], ping_options_t *opts) {
    int opt;
    
    opts->count = DEFAULT_COUNT;
    opts->packet_size = DEFAULT_PACKET_SIZE;
    opts->interval = DEFAULT_INTERVAL;
    opts->ttl = 64;
    opts->timeout = DEFAULT_TIMEOUT;
    opts->deadline = 0;
    opts->quiet = 0;
    memset(opts->source_ip, 0, sizeof(opts->source_ip));
    memset(opts->payload_pattern, 0, sizeof(opts->payload_pattern));
    
    while ((opt = getopt(argc, argv, "c:s:i:t:W:w:q")) != -1) 
    {
        switch (opt) 
        {
            case 'c':
                opts->count = atoi(optarg);
                break;
            case 's':
                opts->packet_size = atoi(optarg);
                break;
            case 'i':
                opts->interval = atof(optarg);
                break;
            case 't':
                opts->ttl = atoi(optarg);
                break;
            case 'W':
                opts->timeout = atoi(optarg);
                break;
            case 'w':
                opts->deadline = atoi(optarg);
                break;
            case 'q':
                opts->quiet = 1;
                break;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    
    if (optind >= argc) 
    {
        fprintf(stderr, "ping: missing host operand\n");
        print_usage(argv[0]);
        return -1;
    }
    
    strncpy(opts->host, argv[optind], sizeof(opts->host) - 1);
    return 0;
}

// Print Statistics
void print_stats(ping_stats_t *stats, struct sockaddr_in *dest) 
{
    double elapsed = (stats->end_time.tv_sec - stats->start_time.tv_sec) +
                     (stats->end_time.tv_usec - stats->start_time.tv_usec) / 1000000.0;
    
    double loss = (stats->sent > 0) ? 
                  (stats->sent - stats->received) * 100.0 / stats->sent : 0;
    
    printf("\n--- %s ping statistics ---\n", inet_ntoa(dest->sin_addr));
    printf("%d packets transmitted, %d received, %.1f%% packet loss, time %.0fms\n",
           stats->sent, stats->received, loss, elapsed * 1000);
    
    if (stats->received > 0) 
    {
        printf("rtt min/avg/max = %.3f/%.3f/%.3f ms\n",
               stats->min_rtt,
               stats->total_rtt / stats->received,
               stats->max_rtt);
    }
}

// MAIN FUNCTION--------------------------------------------------------
int main(int argc, char *argv[]) 
{
    ping_options_t opts;
    struct sockaddr_in dest;
    
    if (parse_args(argc, argv, &opts) < 0) 
    {
        return 1;
    }
    
    signal(SIGINT, handle_signal);
    
    int sock = create_raw_socket();

    if (sock < 0) 
    {
        return 1;
    }
    
    if (resolve_host(opts.host, &dest) < 0) 
    {
        close(sock);
        return 1;
    }
    
    int total_size = opts.packet_size + ICMP_HEADER_SIZE + sizeof(struct timeval);
    
    if (!opts.quiet) 
    {
        printf("PING %s (%s) %d(%d) bytes of data.\n",
               opts.host, inet_ntoa(dest.sin_addr),
               opts.packet_size,
               total_size + 20);
    }
    
    struct timeval timeout;
    timeout.tv_sec = opts.timeout;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    gettimeofday(&stats.start_time, NULL);
    
    int seq = 1;
    struct timeval last_send = {0, 0};
    
    while (running && seq <= opts.count) 
    {
        if (opts.deadline > 0) 
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            double elapsed = (now.tv_sec - stats.start_time.tv_sec) +
                             (now.tv_usec - stats.start_time.tv_usec) / 1000000.0;
            if (elapsed >= opts.deadline) break;
        }
        
        if (last_send.tv_sec != 0 && seq > 1) 
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            double elapsed = (now.tv_sec - last_send.tv_sec) +
                             (now.tv_usec - last_send.tv_usec) / 1000000.0;
            if (elapsed < opts.interval) {
                usleep((opts.interval - elapsed) * 1000000);
            }
        }
        
        char packet[total_size];
        create_icmp_packet(packet, seq, total_size, &opts);
        
        if (sendto(sock, packet, total_size, 0,
            (struct sockaddr *)&dest, sizeof(dest)) <= 0) 
        {
            perror("ping: sendto");
            continue;
        }
        
        stats.sent++;
        gettimeofday(&last_send, NULL);
        
        char recv_buffer[512];
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);
        
        int bytes = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0,
                             (struct sockaddr *)&sender, &sender_len);
        
        if (bytes > 0) 
        {
            struct iphdr *ip_hdr = (struct iphdr *)recv_buffer;
            int ip_hdr_len = ip_hdr->ihl * 4;
            struct icmp *icmp_hdr = (struct icmp *)(recv_buffer + ip_hdr_len);
            
            if (icmp_hdr->icmp_type == ICMP_ECHOREPLY &&
                icmp_hdr->icmp_id == (getpid() & 0xFFFF) &&
                icmp_hdr->icmp_seq == seq) 
                {
                
                stats.received++;
                
                struct timeval *tv_sent = (struct timeval *)((char *)icmp_hdr + ICMP_HEADER_SIZE);
                struct timeval tv_now;
                gettimeofday(&tv_now, NULL);
                
                double rtt = (tv_now.tv_sec - tv_sent->tv_sec) * 1000.0 +
                             (tv_now.tv_usec - tv_sent->tv_usec) / 1000.0;
                
                if (rtt < stats.min_rtt) stats.min_rtt = rtt;
                if (rtt > stats.max_rtt) stats.max_rtt = rtt;
                stats.total_rtt += rtt;
                
                if (!opts.quiet) 
                {
                    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
                           bytes - ip_hdr_len,
                           inet_ntoa(sender.sin_addr),
                           icmp_hdr->icmp_seq,
                           ip_hdr->ttl,
                           rtt);
                }
            }
        } 
        else 
        {
            if (!opts.quiet) 
            {
                printf("Request timeout for icmp_seq=%d\n", seq);
            }
        }
        
        seq++;
    }
    
    gettimeofday(&stats.end_time, NULL);
    print_stats(&stats, &dest);
    
    close(sock);
    return 0;
}
