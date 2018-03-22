#   Server.py
#   Receives instructions via serial from controller (client)
#       instruction     action
#       "F 1\n"         move fwd 1 meter; F,B,R,L
#       "A 30 80\n"     move arm joints to angles
#       "G\n"           move fwd 1 meter
import socket
import time
import serial

#   create socket obj
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#   get local machine name
#host = socket.gethostname()
host = '192.168.1.165'
port = 12345

#   bind port
serversocket.bind((host, port))

#queue up to 5 requests
serversocket.listen(5)

while True:
    #   establish connection
    s,addr = serversocket.accept()
    
    print("Got a connection from %s" % str(addr))
    currentTime = time.ctime(time.time())+"\r\n"
    s.send(currentTime.encode('ascii'))
    question = "press the button"
    s.send(question)
    reply = s.recv(1024)
    print("Reply from client: ")
    print(reply.decode())

    s.close()

    ser = serial.Serial('/dev/ttyACM0', 9600)
    while true:
            print(ser.readline())
