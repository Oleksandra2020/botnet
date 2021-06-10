#!/bin/sh

# shellcheck disable=SC2039
for i in {1..1000}; do
        sleep 0.1;
        ./build/botnet client $(( ${i} + 8200 )) <server_IP> 3916 &
done
