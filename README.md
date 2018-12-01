# MobileArmRobot
A robot with a two joint arm that retrieves pre-marked objects.

Below is the prototype that was evaluated at the end of our final project course. 

Firstly one of two switches is selected. The switches are connected to a RaspberryPi which acts as a server that the Pi on the robot connects to over wifi (python socket programming). The two switches correspond to two different programmed distances. The server Pi sends the command to move the certain distance to the target. 

The robot pi(robot.py script) receives the command and in turn issues a move command to the Arduino controlling the platform. The Arduino implements a control system to determine the distance travelled. 

Once the required distance has been reached, the Arduino uses a proportional control system to mvoe the two joints on the arm to a preprogrammed position. 

Once the motor potentiometers reach the right position, the gripper motor is activated. The Arduino then disengages the arm and moves the robot back the required distance to the point of origin.  

[![Presentation](https://img.youtube.com/vi/_rx5fLndBoY/0.jpg)](https://www.youtube.com/watch?v=_rx5fLndBoY)
https://www.youtube.com/watch?v=_rx5fLndBoY
[![Presentation](https://img.youtube.com/vi/LLRKCYidRPY/0.jpg)](https://www.youtube.com/watch?v=LLRKCYidRPY)
https://www.youtube.com/watch?v=LLRKCYidRPY


The original plan of operation:

Controller consists of switches corresponding to which room, and which object. There are two rooms, with two objects in each room. The objects are a red, and green cup. The Raspberry pi on the controller translates the switches into a message which is then sent over wifi (python socket programming).

The robot raspberry pi receives the wifi instructions. The instruction is of the form 'Bedroom Red'. The robot pi then follows a pre-programmed path to the desired location. Once at the correct room, the pi takes a picture via the camera module. A transformation is employed to determine the translation required to place the robot in front of the target, and have the arm in plane and in reach of the arm.

After having placed itself in front of the target, the pi takes another picture. Another transformation is used to determine the target's height from the ground. Since the arm is in plane with the target, the arm must only achieve the required gripper height in order to grasp the object.

Having grasped the target, the robot pi maneuvers to the robot back to the home position. 
The object is then released. 


