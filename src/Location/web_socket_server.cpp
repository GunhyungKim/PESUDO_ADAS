#include "web_socket_server.hpp"


using namespace std;

namespace socket_server
{
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::server<websocketpp::config::asio> server;

mutex text_mutex;
std::string location_json_text="[]";

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

// Define a callback to handle incoming messages
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;
    
    // check for a special command to instruct the server to stop listening so
    // it can be cleanly exited.
    if (msg->get_payload() == "stop-listening") {
        s->stop_listening();
        return;
    }
    
    try {
        s->send(hdl, location_json_text, msg->get_opcode());
    } catch (websocketpp::exception const & e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

std::string get_message()
{
    text_mutex.lock();
    return location_json_text;
    text_mutex.unlock();
}
void set_message(std::string msg)
{
    text_mutex.lock();
    location_json_text = msg;
    text_mutex.unlock();
}


void* run_server_internal(void *) {
    // Create a server endpoint
    server location_server;

    try {
        // Set logging settings
        location_server.set_access_channels(websocketpp::log::alevel::all);
        location_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        location_server.init_asio();

        // Register our message handler
        location_server.set_message_handler(bind(&on_message,&location_server,_1,websocketpp::lib::placeholders::_2));

        location_server.set_reuse_addr(true);
        // Listen on port 9002
        location_server.listen(9000);

        // Start the server accept loop
        location_server.start_accept();

        // Start the ASIO io_service run loop
        location_server.run();




    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}
pthread_t server_thread;
void run_server()
{
    pthread_create(&server_thread,NULL,run_server_internal,NULL);
}
}