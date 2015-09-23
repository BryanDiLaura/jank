import processing.serial.*;
import processing.opengl.*;
import java.awt.event.*;

/*Initialize Camera Controls*/
PVector position = new PVector(400,400);
PVector movement = new PVector();
PVector rotation = new PVector();
PVector velocity = new PVector();
float movementSpeed = 0.09;
float rotationSpeed = 0.075;
float scaleSpeed = 0.9;
float fScale = 0;
float maxDistance = 30;

/*Initialize Array Building Counters/Arrays*/
int arrayCount = 0;
int maxArray = 200;
int vect_size = 3;
int vect_count = 0;
int row_size = 12;
int row_count = 0;
int col_size = 71;
int col_count = 0;

/*Serial Collection String*/
String inString;

/*Initialize Control Booleans*/
boolean new_Data = false;  //New data flag
boolean leftR = false;  //Scan left to right
boolean poly = false;  //Toggle for polygon view modes
boolean manual = true;  //Toggle switch for entering manual mode
boolean leftCMD = false;  //Command bot to rotate left
boolean rightCMD = false;  //Command bot to rotate right
boolean forwardCMD = false;  //Command bot to move forward
boolean reverseCMD = false;  //Command bot to move in reverse
boolean terminate = false;  //Maxed out the point cloud

/*Polygon Array: Used to draw polygons when polyDraw is called*/ 
PShape[][] polyDraw = new PShape[row_size*col_size][arrayCount+1];

/*Point Cloud: First dimension is the number of arrays that can be scanned, second */
float[][][][] pointCloud = new float[maxArray][row_size][col_size][vect_size];

/*Used to represent zero point*/
float[] zeros = new float[3];

/*Create a serial port object*/
Serial myPort;

//SETUP Serial/GUI
void setup()
{
   //Open window, use OPENGL renderer
   size(800,800,OPENGL);
   //List all the available serial ports
   println(Serial.list());
   //Use first available serial port
   myPort = new Serial(this, Serial.list()[0], 19200);
   //Buffer until a newline character is used
   myPort.bufferUntil('\n');
   // set inital background:
   background(255);
   
   //Initialize zero point
   zeros[0] = 0;
   zeros[1] = 0;
   zeros[2] = 0;
   
   //Make the display window resizable
   if (frame != null) {
     frame.setResizable(true);
   }
   
   //Add a listener for mouse wheel
   addMouseWheelListener(new MouseWheelListener() 
   {
      public void mouseWheelMoved(MouseWheelEvent mwe) 
      {
         mouseWheel(mwe.getWheelRotation());
      }
   });
}

//SWITCH CONTROLS
void keyPressed()
{
   //View controller: View Polygon draw or point cloud
   if (key == 'V' || key == 'v')
   {
    if (!poly) poly = true;
    else poly = false;
   } 
   
  //Switch manual/automatic mode
  if ((key == 'M' || key == 'm'))
  {
    myPort.write('M');
    if (manual) manual = false;
    else manual = true;
  }
  
  //Command forward
  else if ((key == 'W' || key == 'w'))
  {
    myPort.write('W');  //Forward
    forwardCMD = true;
  }
  
  //Command left
  else if ((key == 'A' || key == 'a'))
  {
    myPort.write('A');  //Left turn
    leftCMD = true;
  }
  
  //Command right
  else if ((key == 'D' || key == 'd'))
  {
    myPort.write('D');  //Right turn
    rightCMD = true;
  }
  
  //Command reverse
  else if ((key == 'S' || key == 's'))
  {
    myPort.write('S');  //Reverse
    reverseCMD = true;
  }
  
  //Start a new scan
  else if ((key == 'F' || key == 'f'))
  {
    myPort.write('F');  //SCAN
  }
}

//TERMINATE COMMANDS
void keyReleased()
{ 
  //Terminate forward command
  if ((key == 'W' || key == 'w'))
  {
    myPort.write('w');
    forwardCMD = false;
  }
  
  //Terminate left command
  if ((key == 'A' || key == 'a'))
  {
    myPort.write('a');
    leftCMD = false;
  }
  
  //Terminate right command
  if ((key == 'D' || key == 'd'))
  {
    myPort.write('d');
    rightCMD = false;
  }
  
  //Terminate reverse command
  if ((key == 'S' || key == 's'))
  {
    myPort.write('s');
    reverseCMD = false;
  }
}

//***********************DRAW POLYGONS OR POINTS/GUI HANDLER*****************//
void draw()
{
  //If the mouse is pressed, get the button and add approriate velocity vector to camera positions
  if (mousePressed)
   {
     if (mouseButton==LEFT) 
     {
       velocity.add((pmouseY-mouseY)*0.01,(mouseX-pmouseX)*0.01,0);
     }
     if (mouseButton==RIGHT)
     { 
       movement.add((mouseX-pmouseX)*movementSpeed,(mouseY-pmouseY)*movementSpeed, 0);
     }
  }
  
  //Damp the view scale
  fScale*=(0.85);
  //Damp camera velocity
  velocity.mult(0.85);
  //Add the velocity to the camera rotation
  rotation.add(velocity);
  //Damp movement
  movement.mult(0.85);
  //Add movement to position
  position.add(movement);
  
  //Render lighting
  lights();
  //Background is whitespace
  background(255);
  
  //Thicker stroke for gui
   strokeWeight(4);
   //Draw sidebar
   fill(100,120,200);
   rect(0,0,150, height);
  
  //Thinner stroke weight for controls display
   strokeWeight(2);
   if (manual) 
   {
     //Draw manual/auto button
     fill(255,0,0);
     ellipse(75,75,75,75);
     fill(0);
     text("Manual",55,76);
     
     /*Draw arrow keys and color accordingly*/
     if (forwardCMD){fill(200); strokeWeight(3);}
     else {fill(225); strokeWeight(2);}
     rect(60,150,30,30);
     
     if (leftCMD) {fill(200); strokeWeight(3);}
     else {fill(225); strokeWeight(2);}
     rect(20,190,30,30);
     
     if (rightCMD) {fill(200); strokeWeight(3);}
     else {fill(225); strokeWeight(2);}
     rect(100,190,30,30);
     
     if (reverseCMD) {fill(200); strokeWeight(3);}
     else {fill(225); strokeWeight(2);}
     rect(60,190,30,30);
     
     //Add text to buttons
     strokeWeight(2);
     fill(0);
     text("W",72,167);
     text("A",32,207);
     text("S",72,207);
     text("D",112,207);
   }
   else //Draw placeholder keys if we are not in manual mode
   {
     //Draw manual/auto button
     fill(0,255,0);
     ellipse(75,75,75,75);
     fill(0);
     text("Auto",60,76);
     
     //Draw keys
     fill(255);
     rect(60,150,30,30);
     rect(20,190,30,30);
     rect(100,190,30,30);
     rect(60,190,30,30);
     fill(0);
     text("W",72,167);
     text("A",32,207);
     text("S",72,207);
     text("D",112,207);
   }
   
   //Draw scan button placeholder
   fill(240,240,240);
   rect(20,250,110, 60);
   fill(0);
   text("SCAN:  F", 50,285);
   strokeWeight(1);
  
  //Translate camera position/velocity/rotation
  translate(position.x, position.y, position.z);
  rotateX(rotation.x*rotationSpeed);
  rotateY(rotation.y*rotationSpeed);
  
  
  /*If we are in point cloud mode, iterate and draw points*/
  if (!poly)
  {
    for (int k = 0; k < arrayCount+1; k++)
    {
      for (int i = 0; i < row_size; i++)
      {
        for (int j = 0; j < col_size; j++)
        if (pointCloud[k][i][j][0] != 111111 && pointCloud[k][i][j][0] != 0 && pointCloud[k][i][j][1] != 0 && pointCloud[k][i][j][2] != 0)
        {
          //Use pushmatrix to place points relative to camera position
          pushMatrix();
          strokeWeight(10);
          point(pointCloud[k][i][j][0], -1*pointCloud[k][i][j][1], -1*pointCloud[k][i][j][2]);
          popMatrix();
          //Reset matrix for next point
        }
      }
    }
  }
  
  /*We are in polygon draw mode, call function for number of current scans*/
  else 
  {
   //Ensure that we do not over index our array
   if (arrayCount >= maxArray) arrayCount = maxArray;
   
   //Pass each 3D scan matrix into the draw function to find polygons
   for (int q = 0; q < arrayCount+1; q++)
   {
    fill(93,107,193);
    drawPolygon(pointCloud[q]); 
   }
  }
}



//*****************************SERIAL COMMUNICATIONS HANDLER**************************//
void serialEvent(Serial myPort)
{
  //If we have not reached the end of our full scan matrix
  if (!terminate)
  {
   //RECIEVE DATA FLOAT
   float inData;
   inString = new String(myPort.readBytesUntil('\n'));
   if (inString != null)
   { 
      //Trim whitespace
      inString = trim(inString);
      //Parse float
      inData = Float.parseFloat(inString);
   
   //CHECK TO SEE IF WE ARE DONE SCANNING
   if (inData == 100001)  //Completed a scan, reset parameters
    {
     //Reset parameters and increase our array count
     arrayCount = arrayCount+1;
     row_count = 0;
     col_count = 0;
     vect_count = 0;
     
     new_Data = false;
        
     //Resize poly array
     for (int i = 0;i< (row_size*col_size);i++)
     {
      //If the element is empty, increase array size
      if (polyDraw[i] == null)
       polyDraw[i] = new PShape[arrayCount+1];
      else polyDraw[i] = (PShape[])resizeArray(polyDraw[i],arrayCount+1); //Copy and increase array size of element
     }
     //If we have maximized our array...
     if (arrayCount >= maxArray)
     {
      terminate = true; //Terminate scanning
     }
     myPort.write('G');
     println("Done scan.");
    }
   
   //If we have recieved new data...
    else if (inData == 101010 && !new_Data)
    {
      //Set flag
      new_Data = true;
    }
  
    //Collect float
    else if (new_Data)
    {     
      //Write the float to appropriate vector position
      pointCloud[arrayCount][row_count][col_count][vect_count] = inData;
      println(inData);
      //Increase the count
      vect_count++;
      
      //If we have maximized the vector, increase/decrease the columns accordingly and proceed
      if (vect_count == vect_size) 
      {
        vect_count = 0;
        new_Data = false;
        if (!leftR) col_count++;
        else if (leftR) col_count--;
      }
      
      //If we have maximized columns, switch to left to right mode and increase rows
      if (col_count == col_size && !leftR)  
      {
        leftR = true;
        col_count--;
        row_count++; 
      }
      
      //If we have minimized colums, switch to right to left mode and increase rows
      if (col_count == -1 && leftR)
      {
       col_count++;
       leftR = false;
       row_count++; 
      }
      
      //If we have reached the end of our rows, reset parameters and increase array sizes
      if (row_count == row_size)
      {
        //Reallocate array
        arrayCount++;
        row_count = 0;
        col_count = 0;
        vect_count = 0;
        new_Data = false;
        
        //Resize poly array
        for (int i = 0;i< (row_size*col_size);i++)
        {
         if (polyDraw[i] == null)
          polyDraw[i] = new PShape[arrayCount+1];
         else polyDraw[i] = (PShape[])resizeArray(polyDraw[i],arrayCount+1); 
        }
        
        if (arrayCount >= maxArray)  //If we have reached the end of our scan...
        {
          terminate = true;   //Terminate scanning
        }
        
        println("Done scan.");
        myPort.write('G');
        return;
      }
    }
  }
  //Scan stabilized: Proceed to next data point
  myPort.write('G');
 }
}

//Mouse wheel function
void mouseWheel(int delta) 
{
  //Change the scale of the draw window
   fScale -= delta * scaleSpeed;
   //Increase/decrease the scale
   movement.add(0,0,fScale);
}

//Resizing array: Used online resources to determine appropriate method for java array copying
private static Object resizeArray (Object inArray, int inSize) 
{
   //Get the size of the previous array
   int prevSize = java.lang.reflect.Array.getLength(inArray);
   //Get the type of the array
   Class Type = inArray.getClass().getComponentType();
   //Give the new array the same type
   Object newArray = java.lang.reflect.Array.newInstance(Type, inSize);
   //Determine the minimum size, whether it be the current size of the previous length
   int hold_length = Math.min(prevSize, inSize);
   //Assuming we aren't negative...
   if (hold_length > 0)
      //Copy the old array into the new
      System.arraycopy(inArray, 0, newArray, 0, hold_length);
      //Return the new array
   return newArray; 
 }

//Acquire distance between two vectors 
float distance(float[] p1, float[] p2)
{
  //Return the normal of the difference of the vectors
  return abs(sqrt(((p2[0]-p1[0])*(p2[0]-p1[0]))+((p2[1]-p1[1])*(p2[1]-p1[1]))+((p2[2]-p1[2])*(p2[2]-p1[2]))));
}


//Draw polygons from array clouds
void drawPolygon(float[][][] Cloud)
{
  //Booleans for draw triangles
  boolean ftri_1 = true;
  boolean ftri_3 = true;
  boolean ftri_2 = true;
  boolean ftri_4 = true;

  //Iterate through rows
  for (int i = 0; i < row_size-1; i++)
  {
    //Iterate through columns
    for (int j = 0; j < col_size-1; j++){
      //Initially all triangles are allowed
      ftri_1 = true;
      ftri_2 = true;
      ftri_3 = true;
      ftri_4 = true;
    //Check for zero position or throwout tag
    if (Cloud[i][j][0] == 111111 || (Cloud[i][j][0] == 0 && Cloud[i][j][1] == 0 && Cloud[i][j][2] == 0))      //Check error tag
    {
      //If the point is a throwout then set appropriate triangles to false
      ftri_1 = false;
      ftri_3 = false;
      ftri_4 = false;
    }
    
    //Check procedure for each point of the square
    if (Cloud[i+1][j+1][0] == 111111 || (Cloud[i][j][0] == 0 && Cloud[i][j][1] == 0 && Cloud[i][j][2] == 0))      //Check error tag
    {
      ftri_2 = false;
      ftri_3 = false;
      ftri_4 = false;
    }
    
    if (Cloud[i][j+1][0] == 111111 || (Cloud[i][j][0] == 0 && Cloud[i][j][1] == 0 && Cloud[i][j][2] == 0))      //Check error tag
    {
      ftri_3 = false;
      ftri_1 = false;
      ftri_2 = false;
    }
    
    if (Cloud[i+1][j][0] == 111111 || (Cloud[i][j][0] == 0 && Cloud[i][j][1] == 0 && Cloud[i][j][2] == 0))      //Check error tag
    {
      ftri_4 = false;
      ftri_1 = false;
      ftri_2 = false;
    }
      //Calculate relative distances
      float[] tri_1 = new float[3];
      float[] tri_2 = new float[3];
      float[] tri_3 = new float[3];
      float[] tri_4 = new float[3];
      
      /*Calculate distances using distance function and Cloud vectors*/
      tri_1[0] = distance(Cloud[i][j],Cloud[i+1][j]); 
      tri_1[1] = distance(Cloud[i][j],Cloud[i][j+1]); 
      tri_1[2] = distance(Cloud[i+1][j],Cloud[i][j+1]);
      
      tri_2[0] = distance(Cloud[i+1][j+1],Cloud[i+1][j]); 
      tri_2[1] = distance(Cloud[i+1][j+1],Cloud[i][j+1]); 
      tri_2[2] = distance(Cloud[i+1][j],Cloud[i][j+1]);
      
      tri_3[0] = distance(Cloud[i][j],Cloud[i][j+1]); 
      tri_3[1] = distance(Cloud[i][j+1],Cloud[i+1][j+1]); 
      tri_3[2] = distance(Cloud[i][j],Cloud[i+1][j+1]);
      
      tri_4[0] = distance(Cloud[i][j],Cloud[i+1][j+1]); 
      tri_4[1] = distance(Cloud[i][j],Cloud[i+1][j]); 
      tri_4[2] = distance(Cloud[i+1][j],Cloud[i+1][j+1]);
      
      //Iterate through triangle lengths
      for (int k = 0; k < 3; k++)
      {
        //If a triangle length exceeds a maximum size
         if (tri_1[k] > maxDistance)
         {
           //Set corresponding triangles to false
           ftri_1 = false; 
         }
         
         if (tri_2[k] > maxDistance)
         {
           //Set corresponding triangles to false
           ftri_2 = false; 
         }
         
         if (tri_3[k] > maxDistance)
         {
           //Set corresponding triangles to false
           ftri_3 = false; 
         }
         
         if (tri_4[k] > maxDistance)
         {
           ftri_4 = false;
         }
      }
      
      //If the triangle is still true, draw it
      if (ftri_1 && !(ftri_3 && ftri_4))  //Don't draw triangles 1 & 2 if triangles 3 and 4 are closer
      {
        fill(100);
        //Begin polygon shape
        beginShape();
        //Draw each vertex
        vertex(Cloud[i][j][0], -1*Cloud[i][j][1], -1*Cloud[i][j][2]);
        vertex(Cloud[i+1][j][0], -1*Cloud[i+1][j][1], -1*Cloud[i+1][j][2]);
        vertex(Cloud[i][j+1][0], -1*Cloud[i][j+1][1], -1*Cloud[i][j+1][2]);
        endShape();
        ftri_3 = false;
        ftri_4 = false;
     }
     
     //Check each triangle
     if (ftri_2 && !(ftri_3 && ftri_4))
     {   
        fill(100);
        beginShape();
        vertex(Cloud[i+1][j][0], -1*Cloud[i+1][j][1], -1*Cloud[i+1][j][2]);
        vertex(Cloud[i][j+1][0], -1*Cloud[i][j+1][1], -1*Cloud[i][j+1][2]);
        vertex(Cloud[i+1][j+1][0], -1*Cloud[i+1][j+1][1], -1*Cloud[i+1][j+1][2]);
        endShape();
        ftri_3 = false;
        ftri_4 = false;
     }
      
      if (ftri_3)
      {
        fill(100);
        beginShape();
        vertex(Cloud[i][j][0], -1*Cloud[i][j][1], -1*Cloud[i][j][2]);
        vertex(Cloud[i+1][j+1][0], -1*Cloud[i+1][j+1][1], -1*Cloud[i+1][j+1][2]);
        vertex(Cloud[i][j+1][0], -1*Cloud[i][j+1][1], -1*Cloud[i][j+1][2]);
        endShape();
      }
      if (ftri_4)
      {
        fill(100);
        beginShape();
        vertex(Cloud[i][j][0], -1*Cloud[i][j][1], -1*Cloud[i][j][2]);
        vertex(Cloud[i+1][j][0], -1*Cloud[i+1][j][1], -1*Cloud[i+1][j][2]);
        vertex(Cloud[i+1][j+1][0], -1*Cloud[i+1][j+1][1], -1*Cloud[i+1][j+1][2]);
        endShape();
      }
    }
  }  
}
