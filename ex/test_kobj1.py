#client.py
import socket
import RPi.GPIO as GPIO
import time

#Configure Push Button
GPIO.setmode(GPIO.BCM)

GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(24, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(25, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(16, GPIO.IN, pull_up_down=GPIO.PUD_UP)

while True:
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
	if flag:
		#Socket Programming
		#create a socket object
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

		#get local machine name
		#host = socket.gethostname()
		host = '192.168.43.52'

		port = 12345

		#connection to hostname on the port
		s.connect((host, port))

		#Receive no more than 1024 bytes
		tm = s.recv(1024)
		print(tm)
		question = s.recv(1024)
		print(question.decode())
		ans = "hello"
		ans = str(ans)
		s.send(ans)

		s.close()

		print("The server has successfully executed the command at time %s" % tm.decode('ascii'))

