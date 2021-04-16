#include "../inc/thread_pool.h"
#include "../inc/victim_manipulation.h"

int main(int argc, char *argv[])
{
    victims v;
    v.add_victim("192.168.0.102", "8080", "192.168.0.103", "8080");
    v.add_victim("192.168.0.102", "1024", "192.168.0.104", "1024");
    v.add_victim("192.168.0.102", "1025", "192.168.0.105", "1025");

    return 0;
}
