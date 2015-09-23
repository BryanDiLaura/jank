#include <Stepper.h>
#include <Servo.h>

#define SPEED 20
#define WHEEL_DIAMETER 6
#define ROBOT_DIAMETER 16.8
#define DELAY 20
#define DELAY_BETWEEN_READS 3

boolean collide = 0;

const int stepsPerRevolution = 200;
const float distancePerStep = (PI*WHEEL_DIAMETER)/(float)stepsPerRevolution;
//create stepper objects
Stepper myStepper_right(stepsPerRevolution, 2,3,6,7); 
Stepper myStepper_left(stepsPerRevolution, 8,11,12,13); 

//create servo objects
Servo vertservo;
Servo horservo;

double RL_Movement = 0;  //stores X coordinate
double FB_Movement = 0;  //stores Y coordinate
int robotAngle = 90;
int mode = 0;

void setup() {
  //setup pins
  //INFRARED SENSOR IN ANALOG 0 (A0)
  //VERTICAL SERVO TO PIN 10
  //HORIZONTAL SERVO TO PIN 9
  pinMode(A0, INPUT);
  vertservo.attach(10);
  horservo.attach(9);
  myStepper_left.setSpeed(SPEED);
  myStepper_right.setSpeed(SPEED);
  Serial.begin(9600);
  //get handshake with processing
  Serial.println(1);
  while(Serial.available() == 0);
}


char serRead;
int man = 1;
void loop(){
  
  while(Serial.available() == 0);
  serRead = Serial.read();
  
  if (serRead == 'M'){
    man = !man;
  }
  
  if (serRead == 'W'){
    forward(2);
  }
  
  if (serRead == 'A'){
    left_turn(5);
  }
  
  if (serRead == 'D'){
    right_turn(5);
  }
  
  if (serRead == 'S'){
    backward(2);
  }
  
}
  






void left_turn(int angle){  //angle in degrees
  int steps = angle_to_steps(angle);
  int i;
  for(i=0; i<steps; i++){
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
  for(i=0; i<steps; i++){
    myStepper_left.step(1);
    myStepper_right.step(1);
  }
  robotAngle = robotAngle - angle;
  if(robotAngle < 0) {
    robotAngle = robotAngle + 360;
  }
}

void forward(int dist){  //distance in cm
  int steps = dist_to_steps(dist);
  int i;
  for(i=0; i<steps; i++){
    if((i%20) == 0){
      collide = collision();
    }
    if(collide == 1){
      double distance = steps_to_dist(i);
      RL_Movement = RL_Movement + distance*cos(robotAngle*(PI/180));
      FB_Movement = FB_Movement + distance*sin(robotAngle*(PI/180));
      //sweep();
      mode = 9;
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
double steps_to_dist(int steps){  //passed the steps if its movement is stopped
  float dist = (float) steps*distancePerStep;
  return (double)dist;
}

float dist(float x){
  //convert to voltage
  x = (5.0 / 1024.0)*x;
  float y;
  //compute distance
  y = (306.439 + x * ( -512.611 + x * ( 382.268 + x * (-129.893 + x * 16.2537) ) ));

  return y;
}


boolean collision(){
  boolean collide = 0;
  float distance = 0;
  vertservo.write(130);
  horservo.write(100);
  if(mode == 9) {
    delay(300);
    mode = 0;
  }
  distance = dist(analogRead(A0));
  if(distance <= 65){
    collide = 1; 
  }
  return collide;
}

void sweep(){
  //constants for sweep function
  int vertpos;
  int horpos;
  int lr=0;
  float value = 0;

  int index = 10;
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
        Serial.println(101010);
        //wait for response
        while(Serial.available() == 0);

        //throw out bad data
        if (distance > 170){
          Serial.println(111111);
          Serial.println(111111);
          Serial.println(111111);
        }
        else{
          //send x,y,z (respectively)
          Serial.println((float)xpos(horpos,vertpos,distance));
          Serial.println((float)ypos(vertpos,distance));
          Serial.println((float)zpos(horpos,vertpos,distance));
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
        while(Serial.available() == 0);

        //throw out bad data
        if (distance > 170){
          Serial.println(111111);
          Serial.println(111111);
          Serial.println(111111);
        }
        else{
          //send x,y,z (respectively)
          Serial.println((float)xpos(horpos,vertpos,distance));
          Serial.println((float)ypos(vertpos,distance));
          Serial.println((float)zpos(horpos,vertpos,distance));
        }
      }
      lr=0;
    }
  }
}

// Takes the horizontal and vertical angles and distance to produces an x coordinate.
float xpos(float hangle, float vangle, float dist){
  float theta = (robotAngle-90+(180-(hangle-10))) * PI/180;
  float phi = (vangle-30) * PI/180;
  return dist*cos(theta)*sin(phi) + RL_Movement;
}

// Takes the vertical servo angle and distance and produces a y coordinate.
float ypos(float vangle, float dist){
  float phi = (vangle-30) * PI/180;
  return dist*cos(phi);
}

// Takes the horizontal and vertical angles and distance to produces an z coordinate.
float zpos(float hangle, float vangle, float dist){
  float theta = (robotAngle-90+(180-(hangle-10))) * PI/180;
  float phi = (vangle-30) * PI/180;
  return dist*sin(theta)*sin(phi) + FB_Movement;
}
