#include <Stepper.h>
#include <Servo.h>

#define SPEED 20
#define WHEEL_DIAMETER 6
#define ROBOT_DIAMETER 16.8
#define DELAY 20
#define DELAY_BETWEEN_READS 3
#define SERIAL_DELAY 1
#define LEFT 1
#define CENTER 2
#define RIGHT 3
#define STOPPED 4

//Setup for backup buzzer
#define freq 1100
#define buzz 5

//Command booleans
boolean fCMD = false;
boolean rCMD = false;
boolean lCMD = false;
boolean bCMD = false;
boolean manual = true;
boolean scan = false;
boolean man2auto = true;

volatile boolean collide = 0;
boolean first = 1;

const int stepsPerRevolution = 200;
const float distancePerStep = (PI*WHEEL_DIAMETER)/(float)stepsPerRevolution;
//create stepper objects
Stepper myStepper_right(stepsPerRevolution, 2,3,6,7); 
Stepper myStepper_left(stepsPerRevolution, 8,11,12,13); 

//create servo objects
Servo vertservo;
Servo horservo;

float RL_Movement = 0;  //stores X coordinate
float FB_Movement = 0;  //stores Y coordinate
int robotAngle = 90;
int mode = 0;
char dump;
int reposition = 0;

void setup() 
{
  //setup pins
  //INFRARED SENSOR IN ANALOG 0 (A0)
  //VERTICAL SERVO TO PIN 10
  //HORIZONTAL SERVO TO PIN 9
  pinMode(buzz,OUTPUT);
  pinMode(A0, INPUT);
  vertservo.attach(10);
  horservo.attach(9);
  myStepper_left.setSpeed(SPEED);
  myStepper_right.setSpeed(SPEED);
  Serial.begin(19200);
  //get handshake with processing
  // Serial.println(1);
  // while(Serial.available() == 0);
}

void loop()
{
  if (manual)
  {
    //Manual mode
    if (scan)
    {
      sweep();
      scan = false;
    }
    else if (fCMD) fMan();
    else if (bCMD) bMan();
    else if (rCMD) right_turn(1);
    else if (lCMD) left_turn(1);
    
    man2auto = true;
  }

  else if (!manual)
  {
    if (man2auto)
    {
      delay(5000);
      forward(3000);  //initially move until an object is found
      man2auto = false;
    }

    if((mode == 0) || (mode == STOPPED))
    {
      //follows wall while it won't collide
      if(mode == 0){
        //Serial.println("1");
        right_turn(90);
        //Serial.print("loop: ");
        //Serial.println(collide);
        delay(200);
        forward(40);
        //Serial.print("loop2: ");
        //Serial.println(collide);
      }
      //takes another right turn to avoid collision
      if(mode == STOPPED)
      {
        //Serial.println("2");
        right_turn(90);
        mode = 0;
        forward(40);
      }
    }
    //turns back toward the wall and checks to make sure it is straight ahead
    if(mode == 0)
    {
      //Serial.println("3");
      left_turn(90);
      ocas();
      delay(300);
      //scans if the wall is there as expected
      if(mode == CENTER)
      {
        //Serial.println("4");
        if (!manual)
        {
          sweep();
        }
        mode = 0;
      }
      //if the wall is not straight ahead, but a corner is present, the robot will move forward and turn left to continue following the walls curvature
      else if(mode == LEFT)
      {
        //Serial.println("5");
        forward(50);
        left_turn(90);
        if (!manual)
        {
          sweep();
        }
        mode = 0;
      }
      //if an object is sensed to the right, the robot will move forward, turn right, and scan this object as long as nothing is present to the left and in the center
      else if(mode == RIGHT)
      {
        //Serial.println("6");
        forward(50);
        right_turn(90);
        mode = 0;
      }
      //if there is no object in front, it will move forward in 40cm increments and scan for objects in front of it or on either side
      else
      {
        //Serial.println("7");
        while(mode == 0)
        {
          if (manual)
          {
            break;
          }
          //Serial.println("8");
          forward(40);
          
          //break out of searching for an item if it senses there is a collision straight ahead
          if(mode == STOPPED)
          {
            //Serial.println("9");
            break;
          }
          ocas();
          //ignore items on the right to guarantee taking scans of the item in the center or on the left first and working its way to the right
          if(mode == RIGHT)
          {
            //Serial.println("10");
            mode = 0;
          }
        }
        
        //begin following room again when an object is found to the left
        if(mode == LEFT)
        {
          //Serial.println("11");
          forward(40);
          left_turn(90);
          if (!manual)
          {
            sweep();
          }
          mode = 0;
        }
      }
    }
  }
}

void serialEvent()  //Serial routine to call for commands
{
  char serRead;
  serRead = Serial.read();

  if (serRead == 'F')
  {
    scan = true; 
  }

  //**********************MANUAL MODE SWITCH********************//
  if (serRead == 'M')  //Toggle manual mode
  {
    if (manual)
    {
     manual = false;
    }
    else
    {
      manual = true; 
    }
  }


  //*********************HANDLER FOR GO COMMAND********************//
  if (serRead == 'W'){
    fCMD = true;
  }

  if (serRead == 'A'){
    lCMD = true;
  }

  if (serRead == 'D'){
    rCMD = true;
  }

  if (serRead == 'S'){
    tone(buzz,freq);
    bCMD = true;
  }

  //*****************HANDLER FOR STOP COMMANDS****************//
  if (serRead == 'w'){
    fCMD = false;
  }

  if (serRead == 'a'){
    lCMD = false;
  }

  if (serRead == 'd'){
    rCMD = false;
  }

  if (serRead == 's'){
    noTone(buzz);
    bCMD = false;
  }

}

void left_turn(int angle){  //angle in degrees
  int steps = angle_to_steps(angle);
  int i;
  //performs movements one step at a time to avoid locking up the program
  for(i=0; i<steps; i++)
  {
    serialEvent();
    myStepper_left.step(-1);
    myStepper_right.step(-1);
  }
  robotAngle = robotAngle + angle;
  if(robotAngle >= 360) {
    robotAngle = robotAngle - 360;
  }
}

void right_turn(int angle){  //angle in degrees
  int steps = angle_to_steps(angle);
  int i;
  //performs movements one step at a time to avoid locking up the program
  for(i=0; i<steps; i++)
  {
    serialEvent();
    myStepper_left.step(1);
    myStepper_right.step(1);
  }
  robotAngle = robotAngle - angle;
  if(robotAngle < 0) {
    robotAngle = robotAngle + 360;
  }
}

void fMan()
{
  myStepper_left.step(1);
  myStepper_right.step(-1);
  float distance = steps_to_dist(1);
  RL_Movement = RL_Movement + distance*cos(robotAngle*(PI/180));
  FB_Movement = FB_Movement + distance*sin(robotAngle*(PI/180));
}

void bMan()
{
  myStepper_left.step(-1);
  myStepper_right.step(1);
  float distance = steps_to_dist(1);
  RL_Movement = RL_Movement + distance*cos(robotAngle*(PI/180));
  FB_Movement = FB_Movement + distance*sin(robotAngle*(PI/180));
}

void forward(int dist){  //distance in cm
  int steps = dist_to_steps(dist);
  int i;
  //performs movements one step at a time to avoid locking up the program
  for(i=0; i<steps; i++)
  {
    serialEvent();
    //checks for a collision every 40 steps
    if((i%40) == 0){
      collide = collision();
      // Serial.print("forward: ");
      //Serial.println(collide);
    }
    if(collide == 1)
    {
      float distance = steps_to_dist(i);
      RL_Movement = RL_Movement + distance*cos(robotAngle*(PI/180));
      FB_Movement = FB_Movement + distance*sin(robotAngle*(PI/180));
      if (!manual)
      {
        sweep();
      }
      reposition = 1;
      mode = STOPPED;
      if(first == 1){
        first = 0;
        mode = 0;
      }
      break;
    }
    myStepper_left.step(1);
    myStepper_right.step(-1);
  }
  if(i == steps){
    RL_Movement = RL_Movement + dist*cos(robotAngle*(PI/180));
    FB_Movement = FB_Movement + dist*sin(robotAngle*(PI/180));
  }
}

void backward(int dist){  //distance in cm
  int steps = dist_to_steps(dist);
  int i;
  //performs movements one step at a time to avoid locking up the program
  for(i=0; i<steps; i++){
    myStepper_left.step(-1);
    myStepper_right.step(1);
  }
  RL_Movement = RL_Movement - dist*cos(robotAngle*(PI/180));
  FB_Movement = FB_Movement - dist*sin(robotAngle*(PI/180));
}

int angle_to_steps(int angle){  //passed an angle in degrees and returns the number of steps
  float steps; 
  steps = ((float)angle/360)*(ROBOT_DIAMETER*PI/distancePerStep);
  return (int)steps;
}
int dist_to_steps(int dist){  //passed the distance in cm and returns the number of steps
  float steps = (float)dist/distancePerStep;
  return (int)steps;
}
float steps_to_dist(int steps){  //passed the steps if its movement is stopped
  float dist = (float) steps*distancePerStep;
  return (float)dist;
}


float dist(float x){
  //convert to voltage
  x = (5.0 / 1024.0)*x;
  float y;
  //compute distance
  y = (306.439 + x * ( -512.611 + x * ( 382.268 + x * (-129.893 + x * 16.2537) ) ));

  return y;
}

//centers the rangefinder to check for objects straight ahead while the robot is moving forward
boolean collision(){
  collide = 0;
  //Serial.print("collision: ");
  //Serial.println(collide);
  float distance = 0;
  vertservo.write(125);
  horservo.write(100);
  if(reposition == 1) {
    delay(600);
    reposition = 0;
  }
  //delay(200);
  float value = 0;
  distance = dist(analogRead(A0));
  //Serial.print("Distance: ");
  //Serial.println(distance);
  if(distance <= 65){
    collide = 1; 
  }
  //Serial.print("collision2: ");
  //Serial.println(collide);
  return collide;
}

//this scan is used primarily to make movement decisions
void ocas(){
  int i;
  float distance[2] = {
    0        };
  float coldist[2] = {
    0        };
  vertservo.write(125);
  horservo.write(40);
  delay(600);
  for(i=0;i<3;i++){
    distance[i] = dist(analogRead(A0));
    delay(50);
  }
  coldist[0] = (distance[0]+distance[1]+distance[2])/3;
  horservo.write(160);
  delay(600);
  for(i=0;i<3;i++){
    distance[i] = dist(analogRead(A0));
    delay(50);
  }
  coldist[2] = (distance[0]+distance[1]+distance[2])/3;
  horservo.write(100);
  delay(600);
  for(i=0;i<3;i++){
    distance[i] = dist(analogRead(A0));
    delay(50);
  }
  coldist[1] = (distance[0]+distance[1]+distance[2])/3;
  if(coldist[1] <= 100) mode = 2;
  else if(coldist[0] <= 100) mode = 1;
  else if(coldist[2] <= 100) mode = 3;
  else mode = 0;
  return;
}



//********************SWEEP FUNCTION******************//
void sweep()
{
  //constants for sweep function
  int vertpos;
  int horpos;
  int lr=0;
  float value = 0;

  int index = 20;
  float array [index];

  //sweep up in 5 degree increments at the edges of each lr sweep
  //note 80 was the highest, now setting 100 as top

  for(vertpos = 100; vertpos<=155; vertpos+=5)
  { 
    vertservo.write(vertpos);
    delay(200);

    //sweep right
    if(lr == 0){
      for(horpos = 30; horpos <= 170; horpos+=2){ 

        //set horizontal position and delay
        horservo.write(horpos);
        delay(DELAY);   

        //delay on the first movement
        if (horpos == 30){
          delay(300);
        } 
        value = 0;

        //take in 20 reads and average 
        for (int i = 0; i<index; i++){
          value += dist(analogRead(A0));
          delay(DELAY_BETWEEN_READS);  
        }

        float distance = value/index;

        //send data-ready
        //delay(SERIAL_DELAY);
        Serial.println(101010);
        //wait for response
        while(!Serial.available());
        serialEvent();
        //while(!proceed);    //////////////////////NEW EDIT
        //proceed = false;

        //throw out bad data
        if (distance > 180){
          //delay(SERIAL_DELAY);
          Serial.println(111111);
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println(111111);
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println(111111);
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
        }
        else{
          //send x,y,z (respectively)
          //delay(SERIAL_DELAY);
          Serial.println((float)xpos(horpos,vertpos,distance));
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println((float)ypos(vertpos,distance));
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println((float)zpos(horpos,vertpos,distance));
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
        }
      }
      lr=1;
    }

    //sweep left
    else{
      for(horpos = 170; horpos >= 30; horpos-=2){ 

        //set horizontal position and delay
        horservo.write(horpos);
        delay(DELAY);   

        //delay on the first movement
        if (horpos == 30){
          delay(300);
        } 
        value = 0;

        //take in 20 reads and average 
        for (int i = 0; i<index; i++){
          value += dist(analogRead(A0));
          delay(DELAY_BETWEEN_READS);  
        }

        float distance = value/index;

        //send data-ready
        Serial.println(101010);
        //wait for response
        while(!Serial.available());
        serialEvent();
        //while(!proceed);
        //proceed = false;

        //throw out bad data
        if (distance > 180){
          //delay(SERIAL_DELAY);
          Serial.println(111111);
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println(111111);
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println(111111);
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
        }
        else{
          //send x,y,z (respectively)
          //delay(SERIAL_DELAY);
          Serial.println((float)xpos(horpos,vertpos,distance));
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println((float)ypos(vertpos,distance));
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
          Serial.println((float)zpos(horpos,vertpos,distance));
          while(!Serial.available());
          serialEvent();
          //delay(SERIAL_DELAY);
        }
      }
      lr=0;
    }
  }
  Serial.println(100001);
  
  tone(buzz,freq);
  delay(100);
  noTone(buzz);
  delay(100);
  tone(buzz,freq);
  delay(100);
  noTone(buzz);
}

// Takes the horizontal and vertical angles and distance to produces an x coordinate.
float xpos(float hangle, float vangle, float dist){
  float theta = (robotAngle-90+(180-(hangle-10))) * PI/180;
  float phi = (vangle-35) * PI/180;
  return dist*cos(theta)*sin(phi) + RL_Movement;
}

// Takes the vertical servo angle and distance and produces a y coordinate.
float ypos(float vangle, float dist){
  float phi = (vangle-35) * PI/180;
  return dist*cos(phi);
}

// Takes the horizontal and vertical angles and distance to produces an z coordinate.
float zpos(float hangle, float vangle, float dist){
  float theta = (robotAngle-90+(180-(hangle-10))) * PI/180;
  float phi = (vangle-35) * PI/180;
  return dist*sin(theta)*sin(phi) + FB_Movement;
}



