from socket import *
import json
import web_socket_server
web_socket_server.RunServer()

csock = socket(AF_INET, SOCK_DGRAM)
csock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
csock.bind(('', 4000))

while True:
    s, addr = csock.recvfrom(1024)
    print s
    print addr
    web_socket_server.obj_info_str = s
    #data = json.loads(s)
    

