#client.py
import socket
import RPi.GPIO as GPIO
import time

#Configure Push Button
GPIO.setmode(GPIO.BCM)

'''
GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(24, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(25, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(16, GPIO.IN, pull_up_down=GPIO.PUD_UP)
'''

GPIO.setup(21, GPIO.IN)
GPIO.setup(24, GPIO.IN)

while True:
	if GPIO.input(21):
		time.sleep(2)
		#Socket Programming
		#create a socket object
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		#get local machine name
		#host = socket.gethostname()
		host = '192.168.0.100'
		port = 9000

		#connection to hostname on the port
		s.connect((host, port))

		#Receive no more than 1024 bytes
		#tm = s.recv(1024)
		#print(tm)
		#question = s.recv(1024)
		#print(question.decode())
		#ans = str(ans)
		ans = '1'
		s.send(ans)
		s.close()

		print("The server has successfully executed the command at time ")

	if GPIO.input(24):
		time.sleep(2)
		#Socket Programming
		#create a socket object
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		#get local machine name
		#host = socket.gethostname()
		host = '192.168.0.100'
		port = 9000

		#connection to hostname on the port
		s.connect((host, port))

		#Receive no more than 1024 bytes
		#tm = s.recv(1024)
		#print(tm)
		#question = s.recv(1024)
		#print(question.decode())
		#ans = str(ans)
		ans = '2'
		s.send(ans)
		s.close()

		print("The server has successfully executed the command at time ")

