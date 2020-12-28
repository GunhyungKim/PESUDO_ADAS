import socket
import sys
import gps_rand
from json import dumps
import time
while True:
    # Random GPS adress
    
    Locations = gps_rand.gps_rand()             # Location read
    gps_l = len(Locations)

    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP
    
    server_address = ('localhost', 9037)

    try:
        # Send data
        Locations_s = dumps(Locations)          # transform str
        sent = sock.sendto(Locations_s, server_address)
        print(Locations_s)
    finally:
        print('closing socket')
        sock.close()