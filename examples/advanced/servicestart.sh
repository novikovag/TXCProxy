#!/bin/bash
killall -9 txcproxy.exe
# Finam demo server IP/port
WINEDEBUG=-all wine txcproxy.exe -tq_addr 78.41.194.72 -tq_port 3939 -px_port 4444 &
