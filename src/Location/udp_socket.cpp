#include "udp_socket.hpp"
#define  BUFF_SIZE   1024

using namespace std;

static bool init = false;
static int sock;
static struct sockaddr_in   server_addr;

namespace udp_socket{
    string server_addr_str = "192.168.0.4";
    int server_port = 10000;
    int init_socket()
    {
        init = true;

        int   server_addr_size;
        char   buff_rcv[BUFF_SIZE+5];


        sock  = socket( PF_INET, SOCK_DGRAM, 0);
        
        if( -1 == sock)
        {
            printf( "socket 생성 실패\n");
            return -1;
        }

        memset( &server_addr, 0, sizeof( server_addr));
        server_addr.sin_family     = AF_INET;
        server_addr.sin_port       = htons( server_port);
        server_addr.sin_addr.s_addr= inet_addr( server_addr_str.c_str());
        
        
    }
    void send(string arg)
    {
        if(!init)
            init_socket();
            
        ssize_t r = sendto( sock,  arg.c_str() ,arg.length() , 0,    // +1: NULL까지 포함해서 전송
                        ( struct sockaddr*)&server_addr, sizeof( server_addr)); 
        //cout<<r<<endl;
    }

    void send(vector<location::ObjectLocation> locations)
    {
        send(location::toJson(locations));
    }

    int test5()
    {
        if(init_socket()<0)
            exit(0);
        
        while(1)
        {
            string arg = "hello world";
            send(arg);
        }
        
    }

}
