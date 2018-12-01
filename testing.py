import socket
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

