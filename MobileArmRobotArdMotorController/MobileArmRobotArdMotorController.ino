/*
Platform and Arm Controller

Encoder? HEDS-5000 series counts
ArmJoin0 min, max pos

ArmJoint0 Max voltage = 3.5, min = about 1 volt 
ArmJoint1 Max voltage = 7.1, min = about 1 volt 
Gripper max voltage = 1.3,1.1,1.6(1.33), min = about 0.8
J0 min, max; 70, 960, down to up is lower val to higher val, ccw up = dec pos
J1 min, max; 42, 986, ccw up = dec pos
Gripper min, max: 427, 770, open ,closed

If invert polarity on shaft pot, then result in diff vals

translate; 50500 for 1 meter.
*/

#define MINPWM 100        // At 5.3 volts supplied to the Hbridge, min pwm to move motor
#define MAXPWM 255

#define ARM_JOINT0_ANGLE_FACTOR	1023/180	// Range in pot over range in deg 
#define ARM_JOINT1_ANGLE_FACTOR	1023/180	// Range in pot over range in deg 

#define ARM_JOINT0_HOME_POS 940		// Joint0 and Joint1 are such that arm not extended
#define ARM_JOINT1_HOME_POS 693		// 
#define ARM_GRIPPER_HOME_POS 435	// Gripper is in open position

#define ARM_JOINT0_MIN_POS 70
#define ARM_JOINT0_MAX_POS 960
#define ARM_JOINT1_MIN_POS 42
#define ARM_JOINT1_MAX_POS 986

#define ARM_GRIPPER_MIN_POS 705	// Gripper is in open position
#define ARM_GRIPPER_MAX_POS 960	// Gripper is in closed position

#define ARM_J0_MIN_PWM (255/5)*1
#define ARM_J0_MAX_PWM (255/5)*3.5
#define ARM_J1_MIN_PWM (255/5)*1
#define ARM_J1_MAX_PWM (255/5)*7.1
#define ARM_GRIPPER_MIN_PWM 70   //(255/5)*0.8
#define ARM_GRIPPER_MAX_PWM 80   //(255/5)*1.33
#define CONTROL_SYS_SS_ERROR 50		// The error after which need to engage control sys

#define TRANSLATE_1METER  50500 // Encoder value after having translated 1 meter
#define ROTATE_45DEG 25000      // Encoder value after having rotated 45 degrees

#define TRANSLATE_MAX_PWM 220 
#define TRANSLATE_MIN_PWM 180

#define ROTATE_MAX_PWM 120 
#define ROTATE_MIN_PWM 100	// not sure

struct MOTOR
{
  int motorIN1, motorIN2, motorEN, sensorPin;   // Pins 
  int curShaftPos, curTarget, maxShaftPos, minShaftPos;
  int minPWM, maxPWM, curPWM;        		// saved shaft pos, maxPWM; no higher or unstable/danger
};						// minPWM needs to be where motor just begins to move

//motorIN1, motorIN2, motorEN, sensorPin, shaftPos, curTarget, maxPWM;		
// Platform motors:
MOTOR motor_trans = { 33, 31, 8, 2, 0, 0, 0, 0, 150, 180, 0};        // bwd/fwd
MOTOR motor_rot = { 37, 35, 9, 3, 0, 0, 0, 0, 100, 140, 0};            // rot
// Arm Motors
MOTOR motor_armJoint0 = { 41, 39, 4, A11, 0, ARM_JOINT0_HOME_POS, ARM_JOINT0_MAX_POS, ARM_JOINT0_MIN_POS, ARM_J0_MIN_PWM, ARM_J0_MAX_PWM, 0};
MOTOR motor_armJoint1 = { 49, 47, 6, A12, 0, ARM_JOINT1_HOME_POS, ARM_JOINT0_MAX_POS, ARM_JOINT0_MIN_POS, ARM_J1_MIN_PWM, ARM_J1_MAX_PWM, 0};//110
MOTOR motor_gripper = { 45, 43, 5, A10, 0, ARM_GRIPPER_HOME_POS, ARM_GRIPPER_MAX_POS, ARM_GRIPPER_MIN_POS, ARM_GRIPPER_MIN_PWM, ARM_GRIPPER_MAX_PWM, 0};         	   // gripper motor

int operatingMode = 0;		// Determines what instructions to execute
int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
int joint0Pos = 700, joint1Pos = 400;
unsigned long int curtime = 0;
unsigned long int lastTime= 0;
unsigned long int translationEncoder = 0, rotationEncoder = 0;      // Summation of encoder change, trans; 50500 = 1m, rot; 25000 = 45deg

void incTransEncoder()
{
  translationEncoder++;
}
void incRotEncoder()
{
  rotationEncoder++;
}

int updatePID(int sensorValue, int target)		// Compute PWM pulse from error
{
  int PWM = MINPWM + (MAXPWM - MINPWM)*(abs(target-sensorValue)/1023);
  
  return PWM;
}

int calcMotorPWM(MOTOR a)		// Compute PWM pulse from error
{
  return a.minPWM + (a.maxPWM - a.minPWM)*(abs(a.curTarget-a.curShaftPos)/1023);
}

void motorBwd(MOTOR a, int PWMval)
{
  // Go forward
  digitalWrite(a.motorIN1, HIGH); 
  digitalWrite(a.motorIN2, LOW);
  analogWrite(a.motorEN, PWMval);
}

void motorFwd(MOTOR a, int PWMval)
{
  // Go bkwd
  digitalWrite(a.motorIN1, LOW); 
  digitalWrite(a.motorIN2, HIGH);
  analogWrite(a.motorEN, PWMval);
}

void motorStop(MOTOR a)
{
  digitalWrite(a.motorIN1, HIGH); 
  digitalWrite(a.motorIN2, HIGH);
}

void runMotorFor(int mSec, MOTOR a, bool CCW)
{
	//motorFwd(
	delay(mSec);
	motorStop(a);
}

void updateMotorShaftPos(MOTOR *a)
{
	a->curShaftPos = analogRead(a->sensorPin);
}

/*
*

 Incremental advance limits; tend to oscillate below 550
  optimal operating range; -700, past here and get some strange runaway to max pwm, but motor doesnt move..
                             620(oscillations but reaches) - 650-680     motor reaches, then slips down? then runaway 
                           operating range: 900? - 640

*/

int jointMotorControl(MOTOR a)        // Activating when error > 0? na, should be >10, since made it at 10.. but then wont maintain..
{                                     // check to see if arduino common ground was connected during testing, if it affects perf of j0
                                      // make sure only turning one direction, no motorFwd..
	
}

int moveJointToPos(MOTOR *joint, int pos)		// pos is 0-1023
{
	bool targetFound = false;
	joint->curTarget = pos;
	int pos_Error = joint->curTarget - joint->curShaftPos; 
	if(abs(pos_Error) > 100)
	  {
	     if(joint->curShaftPos > joint->curTarget)     // Need to move upward toward target
		    joint->curPWM += 10;
	     else
		    joint->curPWM -= 2;		// Seems that when overshoot, error under 100
	  }
	  else
	  {
	    if(abs(pos_Error) > 70)
	    {
	      if(joint->curShaftPos > joint->curTarget)     // Need to move upward toward target
		      joint->curPWM += 3;
	      else
		      joint->curPWM -= 1;                  // Fall is lower than rise since not against gravity, dont fall too far away
	    }
	    else
	    {
	      if(abs(pos_Error) > 40)     // Needs correction
	      {
      		if(joint->curShaftPos > joint->curTarget)
      		  joint->curPWM += 2;
      		else
      		  joint->curPWM -= 1;
      	 }
	      else
	      {
      		if(abs(pos_Error) > 10)     // Needs correction
      		{
      			if(joint->curShaftPos > joint->curTarget)
      				joint->curPWM += 1;
      			else
      				joint->curPWM -= 1;
      		}
		      else
			      targetFound = true;	
	      }
	    }
	  }

// Need to check if works; needs adjustment
/*
	if(joint->PWM > joint->maxPWM)			// Constrain
		joint->PWM = joint->maxPWM;
	else if(joint->PWM < 0)
		joint->PWM = 0;
*/

  if(joint->curPWM > 255)      // Constrain
    joint->curPWM = 255;
  else if(joint->curPWM < 0)
    joint->curPWM = 0;


	motorBwd(*joint, joint->curPWM);	
	return targetFound ? 1 : 0;			// if targetFound = true, return 1
}

int motorControl(MOTOR a)	// returns 1 if control engaged, 0 target reached
{	
	if(abs(a.curTarget-a.curShaftPos) > CONTROL_SYS_SS_ERROR)// engage control sys if >e_ss 
	{
		Serial.print("\tAcquiring Target:");
		Serial.print("\n");
		Serial.println(abs(a.curTarget-a.curShaftPos));
		//Serial.println(sensorValue);

		if(a.curTarget-a.curShaftPos > 0)            // if target higher than potValue
			motorFwd(a, calcMotorPWM(a));
		else
			motorBwd(a, calcMotorPWM(a));
		return 1;
	}
	else
	{
		Serial.print("\n Target reached.");      
		motorStop(a);
		return 0;
	}
}

int gripperControl(MOTOR *a, bool open)	// open = true;mv gripper to max pos, return 0; Acquiring target, 1: target found
{						// Check error tolerance; when was at 20, saw motor tried to push past open..
						// add feature to close at certain dist from closed
	int PWM = 0;
//#define ARM_GRIPPER_MIN_PWM (255/5)*0.8
//#define ARM_GRIPPER_MAX_PWM (255/5)*1.33

  int sensorValue = analogRead(a->sensorPin);

	// moves to max pos;
	switch(open)
	{
	case true:	// move gripper to min pos
		if(abs(sensorValue - ARM_GRIPPER_MIN_POS) > 80)
    {
			if(sensorValue > ARM_GRIPPER_MIN_POS)	// shaft is past the open position  		
			{
				a->curPWM = ARM_GRIPPER_MIN_PWM + (ARM_GRIPPER_MAX_PWM - ARM_GRIPPER_MIN_PWM)*(abs(ARM_GRIPPER_MIN_POS-sensorValue)/1023.0);

        if(a->curPWM > 240)     // Constrain
          a->curPWM = 240;
        else if(PWM < 0)
          a->curPWM = 0;

				motorBwd(*a, a->curPWM); // moves toward open pos
        
        return 0;               // Indicate acquiring target
			}
    }
    else
    {
      motorBwd(*a, 0); // Stop

      return 1;               // Indicate target reached
    }
	break;
	case false:
		if(abs(sensorValue - ARM_GRIPPER_MAX_POS) > 80)
    {
			if(sensorValue < ARM_GRIPPER_MAX_POS)	// shaft is below the closed position  		
			{
				a->curPWM = ARM_GRIPPER_MIN_PWM + (ARM_GRIPPER_MAX_PWM - ARM_GRIPPER_MIN_PWM)*(abs(ARM_GRIPPER_MAX_POS-sensorValue)/1023.0);

        
        if(a->curPWM > 240)     // Constrain
          a->curPWM = 240;
        else if(PWM < 0)
          a->curPWM = 0;
				
				motorFwd(*a, a->curPWM); // moves toward closed pos

        return 0;               // Indicate acquiring target
			}
    }
    else
    {
      motorFwd(*a, 0);
    
      return 1;               // Indicate target reached
    }
	break;
	}
}

void setup() 
{
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 

  pinMode(13, OUTPUT); 
  
  // Setup motor conrol I/O pins 
  // Movement:
  pinMode(motor_trans.motorIN1, OUTPUT); 
  pinMode(motor_trans.motorIN2, OUTPUT); 
  pinMode(motor_trans.motorEN, OUTPUT); 
  pinMode(motor_trans.sensorPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(motor_trans.sensorPin),incTransEncoder,CHANGE);

  pinMode(motor_rot.motorIN1, OUTPUT); 
  pinMode(motor_rot.motorIN2, OUTPUT); 
  pinMode(motor_rot.motorEN, OUTPUT); 
  pinMode(motor_rot.sensorPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(motor_rot.sensorPin),incRotEncoder,CHANGE);

  // Arm
  pinMode(motor_armJoint0.motorIN1, OUTPUT); 
  pinMode(motor_armJoint0.motorIN2, OUTPUT); 
  pinMode(motor_armJoint0.motorEN, OUTPUT); 
  pinMode(motor_armJoint0.sensorPin, INPUT); 

  pinMode(motor_armJoint1.motorIN1, OUTPUT); 
  pinMode(motor_armJoint1.motorIN2, OUTPUT); 
  pinMode(motor_armJoint1.motorEN, OUTPUT); 
  pinMode(motor_armJoint1.sensorPin, INPUT); 

  pinMode(motor_gripper.motorIN1, OUTPUT); 
  pinMode(motor_gripper.motorIN2, OUTPUT); 
  pinMode(motor_gripper.motorEN, OUTPUT); 
  pinMode(motor_gripper.sensorPin, INPUT); 

}
/*
void serialEvent() 
{
  while (Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
*/
bool gripperAction = false;
int i1 = 0, i2 = 0;
int distToTravelFactor = 0;

void loop() 
{	
	switch(operatingMode)
	{
		case 0:		// The robot is awaiting instructions
			// Arm motors at home positions, platform not mving
			// waiting for instructions: send 20 as in waiting
			// Once receive inst, send 1 to indicate done mode0
			
			digitalWrite(13, LOW);
			if(Serial.available())
			{  
				distToTravelFactor = Serial.parseInt();
				if(distToTravelFactor == 2)	// waits 1sec for an int	
					{
					  //Serial.print("Received 3\n");
					   //Serial.write("Done\n");
					   digitalWrite(13, HIGH);
             while(!gripperControl(&motor_gripper, true))
					   delay(2000);
					   operatingMode = 1;
					// Open gripper in preparation for coming grasp
					}
				else if(distToTravelFactor == 1)
				{
					digitalWrite(13, HIGH);
					while(!gripperControl(&motor_gripper, true))
					delay(2000);
					operatingMode = 1;
					// Open gripper in preparation for coming grasp
					gripperControl(&motor_gripper, true);
				} 
			}
		break;
		case 1:		// Received instructions, move to target
			// Execute movement commands to mv to loc	
			// moving: send 21; indicates 2: operation incomplete, 1: operating in mode 1
			// if(recvd == 11) go to loc 1
			// elseif (recv == 12) go to loc 2
			// done move, send 1 indicate done mode1, going to mode 2

			// Move forward 1 m, when done, goto opmode2, which is rotate
			if(translationEncoder < TRANSLATE_1METER*distToTravelFactor)
			{
				motorFwd(motor_trans, TRANSLATE_MAX_PWM);
			}
			else
			{
				motorFwd(motor_trans, 0);
       translationEncoder = 0;
				operatingMode = 3;
				// Detach intterupt? bcs no longer need it
			}
			//motorControl(motor_rot);
		break;
		case 2:		// Moved to in front of target, awaiting angles to place gipper
			// Arm motors at home pos, platform not moving
			// 

			// Rotate 45 deg,when done goto opmode3, which is arm 
			if(rotationEncoder < ROTATE_45DEG)
			{
				motorBwd(motor_rot, ROTATE_MAX_PWM);
			}
			else
			{
				motorBwd(motor_rot, 0);
				operatingMode = 3;
				// Detach intterupt? bcs no longer need it
			}
		break;
		case 3:		// Received angle commands
			// Move arm to required angles
			joint0Pos = 720;
			joint1Pos = 450;    // 380 -
	   
			motor_armJoint0.curTarget = joint0Pos;
			motor_armJoint1.curTarget = joint1Pos;
			
			//updateMotorShaftPos(motor_trans);
			//updateMotorShaftPos(motor_rot);
			updateMotorShaftPos(&motor_armJoint0);
			updateMotorShaftPos(&motor_armJoint1);
			//updateMotorShaftPos(motor_gripper);

			Serial.print("\tPWM vals (J0, J1): ");
			Serial.print(motor_armJoint0.curPWM); 
			Serial.print(",");
			Serial.println(motor_armJoint1.curPWM);
			      
		     // delay(100);

			if(moveJointToPos(&motor_armJoint1, joint1Pos))
			{
			  Serial.print("\t<---Joint 1 Target Found. Error: ");
			  Serial.println(abs(motor_armJoint1.curTarget-motor_armJoint1.curShaftPos)); 

			  if(moveJointToPos(&motor_armJoint0, joint0Pos))
			  {
			    Serial.print("\t<---Joint 0 Target Found. Error: ");
			    Serial.println(abs(motor_armJoint0.curTarget-motor_armJoint0.curShaftPos)); 
			  }
			  else
			  {
			    Serial.print("\tJoint 0 Acquiring Target. Error: ");
			    Serial.println(abs(motor_armJoint0.curTarget-motor_armJoint0.curShaftPos));
			    //Serial.println(sensorValue);
			  }
			}
			else
			{
			  Serial.print("\tJoint 1 Acquiring Target. Error: ");
			  Serial.println(abs(motor_armJoint1.curTarget-motor_armJoint1.curShaftPos));
			  //Serial.println(sensorValue);
			}  

			if((abs(motor_armJoint1.curTarget-motor_armJoint1.curShaftPos) + abs(motor_armJoint0.curTarget-motor_armJoint0.curShaftPos)) < 20)
        {
          while(!gripperControl(&motor_gripper, false))
          {
            //moveJointToPos(&motor_armJoint1, joint1Pos);
            //moveJointToPos(&motor_armJoint0, joint0Pos);
            Serial.print("\tGrip.");
            Serial.println(abs(motor_gripper.curShaftPos - ARM_GRIPPER_MAX_POS));
            operatingMode = 5;
            if(abs(motor_gripper.curShaftPos - ARM_GRIPPER_MAX_POS) < 250)
              break;
          }
          motorBwd(motor_armJoint0,40);
          motorBwd(motor_armJoint1,50);
          translationEncoder = 0;
          operatingMode = 5;
        }
      /*
			  if(gripperControl(&motor_gripper, false))   // if true, target found
				{
					operatingMode = 5;	// go back to user
					translationEncoder = 0; 
					// Gripped, now slowly disengage arm..dont want platform and arm on: >3amps
					motorBwd(motor_armJoint0,0);
					motorBwd(motor_armJoint1,50);
					
			    		//gripperAction = !gripperAction;                   // change target
				}
       */
			
			delay(20);	// Not neccessary to control evry iteration	
		break;
		case 4:		// Gripper at position after 3
			// Grip

			// Rotate 45 deg,when done goto opmode3, which is arm 
			if(rotationEncoder < ROTATE_45DEG)
			{
				motorFwd(motor_rot, ROTATE_MAX_PWM);
			}
			else
			{
				motorFwd(motor_rot, 0);
				//translationEncoder = 0;
				rotationEncoder = 0;
				operatingMode = 5;
				// Detach intterupt? bcs no longer need it
			}
		break;
		case 5:		// Obj gripped. move back to usr
	     
			// Move forward 1 m, when done, goto opmode2, which is rotate
			if(translationEncoder < TRANSLATE_1METER*distToTravelFactor)
			{
				motorBwd(motor_trans, TRANSLATE_MAX_PWM);
			}
			else
			{
				motorBwd(motor_trans, 0);
				// Detach intterupt? bcs no longer need it
			}
		break;
	}
 }


/*
motorFwd(translational.maxPWM, translational);
  motorFwd(rotational.maxPWM, rotational);
  //motorFwd(150);          // 150 at 12V PCPSU; translation should be about max velocity
  delay(800);
  motorBwd(rotational.maxPWM, rotational);
  delay(400);
  //motorBwd(150);          // 150 @ 12V PCPSU; rot is too fast
  motorBwd(translational.maxPWM, translational);
  motorFwd(rotational.maxPWM, rotational);
  delay(1200);
  motorStop(translational);
  motorStop(rotational);
  delay(1200);

*/
 
/*
  digitalWrite(pinHigh, HIGH);
  digitalWrite(pinLow, LOW);
     
  if(digitalRead(controlPin) == HIGH)                 // Engage control sys or not
    inControl = true;
  else
    inControl = false;

  if(inControl)
  {
    if(sensorValue < 800 && sensorValue > 65)
    {
      if(abs(target-sensorValue) > 50)                // engage control sys when error greater than 50
      {
        Serial.print("\tControl Engaged. Acquiring Target:");
        Serial.println(abs(target-sensorValue));
        motorFwd(100);// 100 is good amt of speed
      }
      else
      {
        Serial.print("\t Target reached.");      
        motorStop();
      }
    }  
  }
  else
  {
     Serial.print("\t Control Disengaged. Sensor value: ");
     Serial.println(sensorValue);
  }
*/
 
/*
#define FORWARD 8#define BACKWARD 2
#define STOP 5#define PWM_PIN_1 9 //speed control pin for motor1
#define PWM_PIN_2 10 ///speed control pin for motor2 
int Motor[2][2] = //two dimensional array{{4 , 5},   //input pin to control Motor1--> Motor[0][0]=4, Motor[0][1]=5{6 , 7},   //input pin to control Motor2--> Motor[1][0]=6, Motor[1][1]=7}; void setup() {  pinMode(PWM_PIN_1, OUTPUT);     pinMode(PWM_PIN_2, OUTPUT);   pinMode(Motor[0][0], OUTPUT);    pinMode(Motor[0][1], OUTPUT);  pinMode(Motor[1][0], OUTPUT);    pinMode(Motor[1][1], OUTPUT);     motor_run(0, FORWARD);    //motor1 forward  motor_run(1, BACKWARD);   //motor2 backward  } void loop() {   //decelerate  for (int  motor_speed=255; motor_speed>=0; motor_speed--) {    analogWrite(PWM_PIN_1, motor_speed);     analogWrite(PWM_PIN_2, motor_speed);    delay(20);  //delay 20 milliseconds  }     //accelerate  for (int  motor_speed=0; motor_speed<=255; motor_speed++) {    analogWrite(PWM_PIN_1, motor_speed);    analogWrite(PWM_PIN_2, motor_speed);    delay(20);   //delay 20 milliseconds  }} void motor_run(int motor, int movement) {  switch (movement) {    case FORWARD:        digitalWrite(Motor[motor][0], HIGH);      digitalWrite(Motor[motor][1], LOW);      break;    case BACKWARD:         digitalWrite(Motor[motor][0], LOW);      digitalWrite(Motor[motor][1], HIGH);;      break;     case STOP:        digitalWrite(Motor[motor][0], LOW);      digitalWrite(Motor[motor][1], LOW);;      break;        }     }

*/

/*
 * dlinkMobileRobotArm
 * pw;mobilerobot
 * 
 */


 
  /*
  // read the analog in value:
  sensorValue = analogRead(analogInPin);            
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 65, 925, 0, 255);  

  // print the results to the serial monitor:
  Serial.print("sensor = " );                       
  Serial.print(sensorValue);      
  Serial.print("\t output = ");      
  Serial.println(outputValue);   

  if(sensorValue < 800 && sensorValue > 65)
  {
    if(abs(target-sensorValue) > 10)
    {
      Serial.print("\t Acquiring Target:");
      Serial.println(abs(target-sensorValue));
      analogWrite(9, 10);
    }
    else
    {
      Serial.print("\t Target reached.");      
      analogWrite(9, 0);
    }
  } 
 */
