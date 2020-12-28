#include <websocketpp/config/asio_no_tls.hpp>
#include "location.hpp"
#include <websocketpp/server.hpp>
#include <mutex>
#include <iostream>
#include <string>
//https://github.com/zaphoyd/websocketpp/blob/master/examples/echo_server/echo_server.cpp

namespace socket_server
{
    void set_message(std::string msg);
    void run_server();
}