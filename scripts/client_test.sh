#!/bin/sh

# shellcheck disable=SC2039
for i in {1..1000}; do
        sleep 0.1;
        sudo ./build/botnet client 192.168.0.103 10000 $(( ${i} + 10 )) &
done