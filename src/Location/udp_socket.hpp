#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include "location.hpp"
#include <vector>
#include <sstream>

namespace udp_socket{
    extern std::string server_addr_str;
    extern int server_port;
    void send(std::string arg);
    void send(std::vector<location::ObjectLocation> locations);
}