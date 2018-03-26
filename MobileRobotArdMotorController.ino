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
*/

#define MINPWM 100        // At 5.3 volts supplied to the Hbridge, min pwm to move motor
#define MAXPWM 255
#define ARM_JOINT0_HOME_POS 940		// Joint0 and Joint1 are such that arm not extended
#define ARM_JOINT1_HOME_POS 693		// 
#define ARM_GRIPPER_HOME_POS 435	// Gripper is in open position
#define ARM_JOINT0_MIN_POS 70
#define ARM_JOINT0_MAX_POS 960
#define ARM_JOINT1_MIN_POS 42
#define ARM_JOINT1_MAX_POS 986
#define ARM_GRIPPER_MIN_POS 427	// Gripper is in open position
#define ARM_GRIPPER_MAX_POS 770	// Gripper is in closed position
#define ARM_J0_MIN_PWM (255/5)*1
#define ARM_J0_MAX_PWM (255/5)*3.5
#define ARM_J1_MIN_PWM (255/5)*1
#define ARM_J1_MAX_PWM (255/5)*7.1
#define ARM_GRIPPER_MIN_PWM (255/5)*0.8
#define ARM_GRIPPER_MAX_PWM (255/5)*1.33
#define CONTROL_SYS_SS_ERROR 50		// The error after which need to engage control sys

struct MOTOR
{
  int motorIN1, motorIN2, motorEN, sensorPin;   // Pins 
  int curShaftPos, curTarget, maxShaftPos, minShaftPos;
  int minPWM, maxPWM;        // saved shaft pos, maxPWM; no higher or unstable/danger
};						// minPWM needs to be where motor just begins to move

//motorIN1, motorIN2, motorEN, sensorPin, shaftPos, curTarget, maxPWM;		
// Platform motors:
MOTOR motor_trans = { 33, 31, 2, A15, 0, 0, 0, 0, 50, 135};        // bwd/fwd
MOTOR motor_rot = { 37, 35, 3, A14, 0, 0, 20, 80};            // rot
// Arm Motors
MOTOR motor_armJoint0 = { 41, 39, 4, A11, 0, ARM_JOINT0_HOME_POS, ARM_JOINT0_MAX_POS, ARM_JOINT0_MIN_POS, ARM_J0_MIN_PWM, ARM_J0_MAX_PWM};
MOTOR motor_armJoint1 = { 45, 43, 5, A12, 0, ARM_JOINT1_HOME_POS, ARM_JOINT0_MAX_POS, ARM_JOINT0_MIN_POS, ARM_J1_MIN_PWM, ARM_J1_MAX_PWM};
MOTOR motor_gripper = { 49, 47, 6, A10, 0, ARM_GRIPPER_HOME_POS, ARM_GRIPPER_MAX_POS, ARM_GRIPPER_MIN_POS, ARM_GRIPPER_MIN_PWM, ARM_GRIPPER_MAX_PWM};         	   // gripper motor

int operatingMode = 0;		// Determines what instructions to execute
int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
int joint0Pos = 675, joint1Pos = 407;
unsigned long int curtime = 0;
unsigned long int lastTime= 0;

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

void updateMotorShaftPos(MOTOR a)
{
	a.curShaftPos = analogRead(a.sensorPin);
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
	int pos_Error = a.curTarget - a.curShaftPos; 
	if(abs(pos_Error) > 100)
	  {
	     if(sensorValue > target)     // Need to move upward toward target
		PWM += 10;
	      else
		PWM -= 10;		// Seems that when overshoot, error under 100
	  }
	  else
	  {
	    if(abs(pos_Error) > 70)
	    {
	      if(sensorValue > target)     // Need to move upward toward target
		PWM += 5;
	      else
		PWM -= 2;                  // Fall is lower than rise since not against gravity, dont fall too far away
	    }
	    else
	    {
	      if(abs(pos_Error) > 40)     // Needs correction
	      {
		if(sensorValue > target)
		  PWM += 3;
		else
		  PWM -= 1;
	      }
	      else
	      {
		if(abs(pos_Error) > 10)     // Needs correction
		{
		  if(sensorValue > target)
		    PWM += 1;
		  else
		    PWM -= 1;
		}
		else
		  Serial.print("\nTarget Found!");
	      }
	    }
	  }

	if(PWM > maxPWM)			// Constrain
		PWM = maxPWM;
	else if(PWM < 0)
		PWM = 0;	
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

void gripperControl(int sensorValue, bool open)	// open = true;mv gripper to max pos
{
	int PWM = 0;
//#define ARM_GRIPPER_MIN_PWM (255/5)*0.8
//#define ARM_GRIPPER_MAX_PWM (255/5)*1.33

int ARM_GRIPPER_MIN_PWM = 100;
int ARM_GRIPPER_MAX_PWM = 110;

	// moves to max pos;
//	motorBwd(updatePID(sensorValue));
	switch(open)
	{
	case true:	// move gripper to min pos
		if(abs(sensorValue - ARM_GRIPPER_MIN_POS) > 20)
			if(sensorValue > ARM_GRIPPER_MIN_POS)	// shaft is past the open position  		
			{
				PWM = ARM_GRIPPER_MIN_PWM + (ARM_GRIPPER_MAX_PWM - ARM_GRIPPER_MIN_PWM)*(abs(ARM_GRIPPER_MIN_POS-sensorValue)/1023.0);

				motorFwd(PWM); // moves toward open pos
			}
	break;
	case false:
		if(abs(sensorValue - ARM_GRIPPER_MAX_POS) > 20)
			if(sensorValue < ARM_GRIPPER_MAX_POS)	// shaft is below the closed position  		
			{
				PWM = ARM_GRIPPER_MIN_PWM + (ARM_GRIPPER_MAX_PWM - ARM_GRIPPER_MIN_PWM)*(abs(ARM_GRIPPER_MAX_POS-sensorValue)/1023.0);

				motorBwd(PWM); // moves toward open pos
			}
	break;
	}

	if(PWM > 240)			// Constrain
		PWM = 240;
	else if(PWM < 0)
		PWM = 0;


}

void setup() 
{
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 
  
  // Setup motor conrol I/O pins 
  // Movement:
  pinMode(motor_trans.motorIN1, OUTPUT); 
  pinMode(motor_trans.motorIN2, OUTPUT); 
  pinMode(motor_trans.motorEN, OUTPUT); 
  pinMode(motor_trans.sensorPin, INPUT); 

  pinMode(motor_rot.motorIN1, OUTPUT); 
  pinMode(motor_rot.motorIN2, OUTPUT); 
  pinMode(motor_rot.motorEN, OUTPUT); 
  pinMode(motor_rot.sensorPin, INPUT); 
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

void loop() 
{
	switch(operatingMode)
	{
		case 0:		// The robot is awaiting instructions
		// Arm motors at home positions, platform not mving
		// waiting for instructions: send 20 as in waiting
		// Once receive inst, send 1 to indicate done mode0
		break;
		case 1:		// Received instructions, move to target
		// Execute movement commands to mv to loc	
		// moving: send 21; indicates 2: operation incomplete, 1: operating in mode 1
		// if(recvd == 11) go to loc 1
		// elseif (recv == 12) go to loc 2
		// done move, send 1 indicate done mode1, going to mode 2
		break;
		case 2:		// Moved to in front of target, awaiting angles to place gipper
		// Arm motors at home pos, platform not moving
		// 
		break;
		case 3:		// Received angle commands
		// Move arm to required angles
			motor_armJoint0.curTarget = joint0Pos;
			motor_armJoint1.curTarget = joint1Pos;
			
			//updateMotorShaftPos(motor_trans);
			//updateMotorShaftPos(motor_rot);
			updateMotorShaftPos(motor_armJoint0);
			updateMotorShaftPos(motor_armJoint1);
			//updateMotorShaftPos(motor_gripper);

			//motorControl(motor_trans);
			//motorControl(motor_rot);
			motorControl(motor_armJoint0);
			motorControl(motor_armJoint1);
			//motorControl(motor_gripper);
			
			delay(20);	// Not neccessary to control evry iteration	
		break;
		case 4:		// Gripper at position after 3
		// Grip
		break;
		case 5:		// Obj gripped. move back to usr
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
