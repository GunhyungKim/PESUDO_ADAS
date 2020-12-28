#-*- coding: utf-8 -*-

#웹에서 요청하는 위치정보 전송 코드
#https://coderwall.com/p/gvknya/python-websocket-server

from tornado import websocket
import tornado.ioloop

import tornado.httpserver
import tornado.websocket
import tornado.ioloop
import tornado.web
import socket
'''
This is a simple Websocket Echo server that uses the Tornado websocket handler.
Please run `pip install tornado` with python of version 2.7.9 or greater to install tornado.
This program will echo back the reverse of whatever it recieves.
Messages are output to the terminal for debuggin purposes. 
''' 
names = ['flare_signal','warning_tripod','dog','cat','deer','pedestrian','car','fire_truck','ambulance']
import json
import socket_udp_server
import threading
from multiprocessing import Process,Value,Manager
import multiprocessing
from ctypes import c_char_p

objList = {"message":"[]"}

def readServerloop(obj):
    global objList
    objList = obj
    print(obj['message'])
    while True:
        objList['message'] = socket_udp_server.GPSdata()
        print (objList['message'])


import threading
from time import sleep
class WSHandler(tornado.websocket.WebSocketHandler):
   
    def sendMsg(self):
       global objList
       message = objList['message']
       self.write_message(message)     

    def open(self):
        #self.sendMsgThread = threading.Thread(target=self.sendMsg,args=(self,))
        #self.sendMsgThread.start()
       # self.pCallback = Periodical
        self.sendMsg()
        print ('new connection')
      
    def on_message(self, message):
        print ('message received:  %s',message)
        # Reverse Message and send it back
        #print ('sending back message: %s', message[::-1])
        print('send data')
        self.sendMsg()
        #self.write_message(message[::-1])
 
    def on_close(self):
        #self.sendMsgThread._stop()
        print ('connection closed')
 
    def check_origin(self, origin):
        return True
 
application = tornado.web.Application([
    (r'/ws', WSHandler),
])
 
 #메인 함수
def ServerThreadMain():
    #t = threading.Thread(target=readServerloop)
    #t.start()

    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(9000)
    #myIP = socket.gethostbyname("192.168.0.4")
    myIP = socket.gethostbyname(socket.gethostname())
    print ('*** Websocket Server Started at %s***', myIP)
    tornado.ioloop.IOLoop.instance().start()


def RunServer(join = False):
    
    t = threading.Thread(target=ServerThreadMain)
    t.start()
    return t


def RunStart(obj):
    global objList
    objList = obj
    print(obj['message'])
    t= RunServer(True)
    t.join()

if __name__ == "__main__":
    d= Manager().dict()
    d['message']= "[]"

    proc1 = Process(target=readServerloop,args=(d,))
    proc2 = Process(target=RunStart,args=(d,))
    proc1.start()
    proc2.start()
    proc1.join()
    proc2.join()