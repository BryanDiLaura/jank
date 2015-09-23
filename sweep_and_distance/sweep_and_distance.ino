//team KICKASS


#include <Servo.h>

//create servo objects
Servo vertservo;
Servo horservo;

//setup constants
#define DELAY 20

void setup(){
 Serial.begin(9600); 
 
 //setup pins
 //INFRARED SENSOR IN ANALOG 0 (A0)
 //VERTICAL SERVO TO PIN 10
 //HORIZONTAL SERVO TO PIN 9
 pinMode(A0, INPUT);
 vertservo.attach(10);
 horservo.attach(9);
 

 
 //get handshake with processing
 Serial.println(1);
 while(Serial.available() == 0);

 sweep(); 
 
 
}



float test;
void loop(){
  
  /*
  test = analogRead(A0);
  Serial.println();
  Serial.print((int)dist(test));
  Serial.print(" cm");
  delay(100);
  */
  
  
 
  
  
}



float dist(float x){
  //convert to voltage
  x = (5.0 / 1024.0)*x;
  float y;
  //compute distance
  y = (306.439 + x * ( -512.611 + x * ( 382.268 + x * (-129.893 + x * 16.2537) ) ));
  
  return y;
}



void sweep(){
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
          delay(3);  
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
          delay(3);  
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
  float theta = (180-(hangle-10)) * PI/180;
  float phi = (vangle-35) * PI/180;
  return dist*cos(theta)*sin(phi);
}

// Takes the vertical servo angle and distance and produces a y coordinate.
float ypos(float vangle, float dist){
  float phi = (vangle-35) * PI/180;
  return dist*cos(phi);
}

// Takes the horizontal and vertical angles and distance to produces an z coordinate.
float zpos(float hangle, float vangle, float dist){
  float theta = (180-(hangle-10)) * PI/180;
  float phi = (vangle-35) * PI/180;
  return dist*sin(theta)*sin(phi);
}
