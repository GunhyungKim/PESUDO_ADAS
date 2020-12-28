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
import json
import random
import threading
from time import sleep

obj_info_str="[]"

class WSHandler(tornado.websocket.WebSocketHandler):

    
    
    def sendMsg(self):
       self.write_message(obj_info_str)
            

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
    
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(9000)
    myIP = socket.gethostbyname(socket.gethostname())
    print ('*** Websocket Server Started at %s***', myIP)
    tornado.ioloop.IOLoop.instance().start()
def RunServer(join = False):
    t = threading.Thread(target=ServerThreadMain)
    t.start()
    return t

if __name__ == "__main__":
    t= RunServer(True)
    t.join()
    