!/bin/sh

for (( i=0; i<4000; i++ )); do 
        ../build/botnet client $(( ${i} + 11000 )) &
done

