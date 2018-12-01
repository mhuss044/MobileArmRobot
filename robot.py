# Python script to orchestrate robot
'''
Acts as a server: receives command from client

'''
import socket
import time
import serial
import RPi.GPIO as GPIO

# create socket obj
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#   get local machine name
#host = socket.gethostname()
host = '192.168.0.100'
port = 9000

#   bind port
serversocket.bind((host, port))

#queue up to 5 requests
serversocket.listen(5)


'''
GPIO.setmode(GPIO.BCM)	# set pinout numbering type

GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(24, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(25, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(16, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(21, GPIO.OUT)
GPIO.setup(20, GPIO.IN)

'''

ser = serial.Serial('/dev/ttyACM0', 9600)   # open serial con with ard
time.sleep(2)       # since after open serial, ard resets, wait before send

    
while True:
	#establish connection:
	s,addr = serversocket.accept()
	
	print("Got a connection from %s\n" % str(addr))
	data = s.recv(1024)
	print(data)
	
	ser.write(str(data))
	print ser.readline()
	s.close()



