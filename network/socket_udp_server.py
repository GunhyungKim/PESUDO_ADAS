import socket
import sys

initialize = False
def init():
    global sock,server_address
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ('localhost', 9037)
    sock.bind(server_address)

def GPSdata():
    global initialize
    # Create a UDP socket
    # Bind the socket to the port
    if(not initialize):
        init()
        initialize = True

    print('starting up on {} port {}'.format(*server_address))
    

    data, address = sock.recvfrom(4096) 
    
    print(data)
    
    return data