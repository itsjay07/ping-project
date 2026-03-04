# 📡 Ping Command

A simple implementation of the Linux ping command in C.

## 🚀 Features
- Send ICMP Echo requests to any host
- Support for -c (count), -s (size), -i (interval)
- Statistics with packet loss and min/avg/max
- Ctrl+C handling with stats display

## 🛠️ Building

```bash
make
sudo ./ping 1.1.1.1
