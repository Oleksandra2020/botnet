#!/bin/sh

# shellcheck disable=SC2039
for i in {1..1000}; do
        sleep 0.1;
        ./build/botnet client <server_IP> 3916 $(( ${i} + 8200 )) &
done
