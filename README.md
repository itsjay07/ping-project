# 🏓 Ping Command – ICMP Echo Implementation in C

<p align="center">
  <img src="https://img.shields.io/badge/version-1.0.0-blue.svg" alt="Version">
  <img src="https://img.shields.io/badge/language-C-00599C.svg" alt="Language">
  <img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License">
  <img src="https://img.shields.io/badge/platform-Linux%20%7C%20WSL-yellow.svg" alt="Platform">
  <img src="https://img.shields.io/badge/network-ICMP-orange.svg" alt="Network">
</p>

<p align="center">
  <b>A complete reimplementation of the Linux <code>ping</code> utility built from scratch in C</b><br>
  ICMP Echo Requests • RTT Calculation • Packet Loss Detection • Network Diagnostics
</p>

---

# 📖 Table of Contents

- [Introduction](#-introduction)
- [What is Ping?](#-what-is-ping)
- [Features](#-features)
- [How It Works](#-how-it-works)
- [Technologies Used](#-technologies-used)
- [Prerequisites](#-prerequisites)
- [Installation](#-installation)
- [Usage Guide](#-usage-guide)
- [Examples](#-examples)
- [Project Structure](#-project-structure)
- [Key Files Explained](#-key-files-explained)
- [How the Code Works](#-how-the-code-works-step-by-step)
- [Common Errors & Solutions](#-common-errors--solutions)
- [Future Improvements](#-future-improvements)
- [FAQ](#-faq)

---

# 📌 Introduction

This project is a **low-level reimplementation of the Linux `ping` command** using raw sockets in C.

It demonstrates deep understanding of:

- ICMP packet construction
- Internet checksum calculation
- Round Trip Time (RTT) measurement
- Packet loss detection
- Signal handling (Ctrl+C)
- DNS resolution
- Command-line parsing using `getopt()`

The goal was to understand how network diagnostic tools work internally at the protocol level.

---

# 🎯 What is Ping?

**Ping (Packet Internet Groper)** is a network diagnostic tool used to test connectivity between two devices.

It works by:

1. Sending an **ICMP Echo Request**
2. Receiving an **ICMP Echo Reply**
3. Measuring the **Round Trip Time (RTT)**

### What Ping Helps You Measure

- ✅ Host availability
- 📊 Network latency
- 📉 Packet loss percentage
- 🌐 Network stability

---

# ✨ Features

| Feature | Description | Example |
|----------|-------------|----------|
| Basic Ping | Send default 4 packets | `./ping 1.1.1.1` |
| Count Option | Specify number of packets | `-c 5` |
| Packet Size | Customize payload size | `-s 100` |
| Interval Control | Set delay between packets | `-i 0.5` |
| TTL Setting | Configure Time To Live | `-t 64` |
| Timeout | Response wait time | `-W 2` |
| Deadline | Total runtime limit | `-w 10` |
| Quiet Mode | Show only statistics | `-q` |
| Statistics | Min/Avg/Max RTT | Automatic |
| Signal Handling | Graceful Ctrl+C exit | Built-in |

---

# 🔧 How It Works

## ICMP Packet Structure

```
IP Header (Handled by Kernel)
└── ICMP Header
    ├── Type (8 = Echo Request)
    ├── Code (0)
    ├── Checksum
    ├── Identifier (Process ID)
    ├── Sequence Number
    ├── Timestamp (RTT calculation)
    └── Payload
```

## Execution Flow

1. Parse CLI arguments (`getopt()`)
2. Create raw socket (`SOCK_RAW`)
3. Resolve hostname to IP
4. Construct ICMP packet
5. Send packet (`sendto()`)
6. Wait for reply (`recvfrom()`)
7. Validate response (Type + Identifier)
8. Compute RTT
9. Update statistics
10. Print summary

---

# 🛠 Technologies Used

- C Programming
- Raw Sockets (`SOCK_RAW`)
- ICMP Protocol
- `getopt()`
- `gettimeofday()`
- `gethostbyname()`
- POSIX Signal Handling
- Makefile Build System

---

# 📦 Prerequisites

- Linux (Ubuntu/Debian) or WSL
- GCC Compiler
- Root / sudo privileges (for raw sockets)

Install required tools:

```bash
sudo apt update
sudo apt install gcc make
```

---

# 🚀 Installation

```bash
git clone https://github.com/itsjay07/ping-project.git
cd ping-project
make clean
make
```

Optional (run without sudo):

```bash
make install
```

---

# 📋 Usage Guide

```
Usage: ./ping [OPTIONS] HOST

Options:
  -c CNT   Number of packets (default: 4)
  -s SIZE  Packet size in bytes (default: 56)
  -i SEC   Interval between packets
  -t TTL   Set Time To Live
  -W SEC   Timeout per reply
  -w SEC   Total runtime limit
  -q       Quiet mode (statistics only)
```

---

# 💡 Examples

```bash
sudo ./ping 1.1.1.1
sudo ./ping -c 3 1.1.1.1
sudo ./ping -s 100 -c 3 1.1.1.1
sudo ./ping -i 0.5 -c 5 1.1.1.1
sudo ./ping -t 128 1.1.1.1
sudo ./ping -q -c 4 1.1.1.1
sudo ./ping -w 5 1.1.1.1
sudo ./ping google.com
```

---

# 📁 Project Structure

```
ping-project/
├── include/
│   └── ping.h
├── src/
│   └── ping.c
├── Makefile
└── README.md
```

---

# 📂 Key Files Explained

| File | Purpose | Key Responsibilities |
|------|---------|---------------------|
| `ping.c` | Core implementation | Socket handling, packet creation, RTT logic |
| `ping.h` | Header definitions | Structures, constants, prototypes |
| `Makefile` | Build automation | Compile, clean, install |

---

# 🔍 How the Code Works (Step-by-Step)

## Statistics Structure

```c
typedef struct {
    int sent;
    int received;
    double min_rtt;
    double max_rtt;
    double total_rtt;
    struct timeval start_time;
    struct timeval end_time;
} ping_stats_t;
```

Tracks sent packets, received packets, and RTT metrics.

---

## Checksum Calculation

```c
unsigned short calculate_checksum(void *b, int len);
```

Uses Internet Checksum (one's complement sum) to ensure packet integrity.

---

## Packet Creation

- Type = `ICMP_ECHO`
- Identifier = `getpid()`
- Sequence number increments per packet
- Timestamp embedded inside payload
- Checksum calculated last

---

## Main Loop

- Send packet
- Wait for reply
- Validate ICMP type and ID
- Calculate RTT
- Update min/avg/max
- Repeat until count reached

---

# 🐛 Common Errors & Solutions

### Operation Not Permitted

```bash
./ping: socket: Operation not permitted
```

**Fix:**
```bash
sudo ./ping 1.1.1.1
```

---

### Unknown Host

Check hostname spelling or network connection.

---

### 100% Packet Loss

Likely firewall blocking ICMP.

---

### Invalid Count

Use positive integers:

```bash
sudo ./ping -c 3 1.1.1.1
```

# 🔍 The Byte / Packet Size Debugging Journey

One of the most challenging parts of this project was matching the **exact output format** of the system `ping` command.

At first glance, everything worked — packets were sent, replies were received, RTT was correct —  
but the **displayed byte sizes didn’t match system ping**.

---

## 🧩 The Problem

My initial output:

```
PING 1.1.1.1 (1.1.1.1) 56(100) bytes of data.
80 bytes from 1.1.1.1: icmp_seq=1 ttl=52 time=59.1 ms
```

System `ping` output:

```
PING 1.1.1.1 (1.1.1.1) 56(84) bytes of data.
64 bytes from 1.1.1.1: icmp_seq=1 ttl=52 time=59.1 ms
```

Everything was consistently **8 bytes larger** in my implementation.

---

## 🕵️ The Investigation

I broke down exactly what was being sent and what was being displayed.

| Component        | Size      | Counted by Me | Counted by System Ping |
|------------------|----------|---------------|------------------------|
| ICMP Header      | 8 bytes  | ✅ Yes        | ✅ Yes                 |
| Payload          | 56 bytes | ✅ Yes        | ✅ Yes                 |
| Timestamp        | 8 bytes  | ✅ Yes        | ❌ No                  |
| IP Header        | 20 bytes | Included      | Included               |

---

## 🎯 The Root Cause

The timestamp is embedded inside the ICMP payload for RTT calculation.

However:

> **System ping does NOT include the timestamp in the displayed byte count.**

Even though the timestamp is physically present in the packet, it is excluded from the printed size.

---

## 📐 The Calculation Difference

### ❌ My Original Calculation

```
Display size = ICMP Header (8)
             + Timestamp (8)
             + Payload (56)
             = 72 bytes
```

Which led to:

```
56(100)
80 bytes from ...
```

---

### ✅ System Ping Calculation

```
Display size = ICMP Header (8)
             + Payload (56)
             = 64 bytes
```

Timestamp is ignored in display size.

---

# 🛠 The Fix

## Header Line Fix

### ❌ Before (Incorrect)

```c
printf("PING %s (%s) %d(%d) bytes of data.\n",
       opts.host, inet_ntoa(dest.sin_addr),
       opts.packet_size,
       icmp_packet_size + 20);  // Produced 56(100)
```

---

### ✅ After (Correct)

```c
printf("PING %s (%s) %d(%d) bytes of data.\n",
       opts.host, inet_ntoa(dest.sin_addr),
       opts.packet_size,
       ICMP_HEADER_SIZE + opts.packet_size + 20);  // Produces 56(84)
```

---

## Response Line Fix

### ❌ Before (Incorrect)

```c
int icmp_recv_size = bytes - ip_hdr_len;  // Produced 80 bytes

printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
       icmp_recv_size, ...);
```

---

### ✅ After (Correct)

```c
int display_size = ICMP_HEADER_SIZE + opts.packet_size;  // Produces 64 bytes

printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.1f ms\n",
       display_size, ...);
```

---

# 💡Byte-Level Clarification

The timestamp is:

- ✅ Required for RTT calculation  
- ✅ Present inside the packet  
- ❌ Not included in the displayed byte count  

That small design detail explains why all my numbers were **exactly 8 bytes too high**.

---

# 🎉 Final Result

After fixing both calculations:

```
PING 1.1.1.1 (1.1.1.1) 56(84) bytes of data.
64 bytes from 1.1.1.1: icmp_seq=1 ttl=52 time=59.1 ms
64 bytes from 1.1.1.1: icmp_seq=2 ttl=52 time=41.4 ms
64 bytes from 1.1.1.1: icmp_seq=3 ttl=52 time=49.1 ms

--- 1.1.1.1 ping statistics ---
4 packets transmitted, 4 received, 0.0% packet loss, time 3043ms
rtt min/avg/max/mdev = 40.167/47.442/59.077/0.000 ms
```

Perfect match with the system `ping` command.

---

## 🧠 What This Taught Me

- Output formatting matters as much as functionality
- Protocol-level accuracy requires understanding implementation details
- Small byte-level assumptions can cause consistent systemic errors
- Reverse-engineering system tools builds deep protocol intuition

This debugging session strengthened my understanding of:

- ICMP packet structure  
- Linux networking behavior  
- Byte-level packet accounting  
- Implementation vs. presentation logic  

---

---

---

# ❓ FAQ

### ❓ Why do I need `sudo`?

Raw sockets require root privileges for security reasons.

You can either run:

```bash
sudo ./ping 1.1.1.1
```

Or install with:

```bash
make install
```

---

### ❓ Why can't I ping localhost (127.0.0.1)?

Some systems disable ICMP responses on `localhost`.

Test with:

```bash
sudo ./ping 1.1.1.1
```

---

### ❓ How is RTT calculated?

A timestamp is stored inside the ICMP payload before sending.

```
RTT = Current Time − Sent Time
```

Displayed in milliseconds with microsecond precision.

---

### ❓ What's a good packet size for testing?

- Default: 56 bytes
- MTU testing: 1472 bytes (before fragmentation on Ethernet)

---

### ❓ Why do I see "Request timeout"?

The host did not respond within the timeout window.

Possible causes:
- Firewall blocking ICMP
- Host offline
- Network congestion

---

### ❓ Can I ping a hostname instead of an IP?

Yes:

```bash
sudo ./ping google.com
```

DNS resolution converts the hostname to an IP before sending packets.

---
