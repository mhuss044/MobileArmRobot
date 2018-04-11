#   Server.py
#   Receives instructions via serial from controller (client)
#       instruction     action
#       "F 1\n"         move fwd 1 meter; F,B,R,L
#       "A 30 80\n"     move arm joints to angles
#       "G\n"           move fwd 1 meter
import socket
import time
import serial
import RPi.GPIO as GPIO

#   create socket obj
#serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#   get local machine name
#host = socket.gethostname()
# 192.168.0.100
host = '192.168.0.100'
port = 12345

#   bind port
#serversocket.bind((host, port))

#queue up to 5 requests
#serversocket.listen(5)

GPIO.setmode(GPIO.BCM)

'''
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(24, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(25, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(16, GPIO.IN, pull_up_down=GPIO.PUD_UP)
'''

GPIO.setup(20, GPIO.IN) # pin 20 as input
ser = serial.Serial('/dev/ttyACM0', 9600)   # open serial con with ard
time.sleep(2)       # since after open serial, ard resets, wait before send
    
while True:
    if GPIO.input(20)   # high on pin 20, go
        ser.write('3')
        while True:
                print ser.readline()


    # while True:
    '''
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
    '''
    #print(serial.tools.list_ports)
    # if button1, then move to room 1
    # if button2, then move to room 2

''' 
		Kitchen = GPIO.input(18)
		flag = False
		if Kitchen == False:
			Kobj1 = GPIO.input(23)
			Kobj2 = GPIO.input(24)
			if Kobj1 == False:
				ans = '0'
				print(ans)
				time.sleep(0.2)
				flag = True
			if Kobj2 == False:
				ans = '1'
				print(ans)
				time.sleep(0.2)
				flag = True
		
		Bedroom = GPIO.input(25)
		if Bedroom == False:
			Bobj1 = GPIO.input(12)
			Bobj2 = GPIO.input(16)
			if Bobj1 == False:
				ans = '2'
				print(ans)
				time.sleep(0.2)
				flag = True
				
			Bobj2 = GPIO.input(16)
			if Kobj2 == False:
				ans = '3'
				print(ans)
				time.sleep(0.2)
				flag = True

'''
