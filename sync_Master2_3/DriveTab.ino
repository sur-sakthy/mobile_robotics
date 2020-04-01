
// Motor Pin Declarations
    const int motor_left_PWM = 34;    // P2.6
    const int motor_right_PWM = 38;   // P2.5
    const int motor_left_phase = 40;  // P2.7
    const int motor_right_phase = 39; // P2.6

// holds sensor values
    int a[5] = {0,0,0,0,0};

//Analog pins           P4.7, P4.4, P4.2, P4.0, P6.1
    int AnalogPin[5] = {A6, A9, A11, A13, A14};  
                                                                 
// PID variables                                                                
    float Kp = 35;         // 50 -> speed=255
    float Ki =  0;        
    float Kd = 10;         // 30
    int pidVal = 0;

// cross track error
    float err = 0;
    float prevErr = 0;

// integral and differential error
    float intErr = 0;
    float diffErr = 0;

// motor speed
    int speed=215;
    int prevSpeed;
    double offsetL=1;
    double offsetR=0.95;

// sensor threshold
    int thres= 100; // tested at 100 -> maybe included varying threshold

// indicates when turning, helpes bring car back to line if it deviates
    boolean hardTurn = false;

// ir sensor pin
    int sensorpin=2;
    int val=0;

// north =  1 -> robot facing 5/north
// north = -1 -> robot facing away from 5/south
    int north= -1; // starting direction
 


void setup()
{
 // put your setup code here, to run once:
  Serial.begin(115200);
 //forward();
}



void loop(){
      if(!hold){ hold=true;  nextpos(pos, nextPos); }
}
 
 
void PID(){

 // stop if obstice in the way   
    if(analogRead(sensorpin)>650){

          blocked=255;
          // stop         
          analogWrite(motor_right_PWM,0);
          analogWrite(motor_left_PWM, 0);  
          
          // wait for path to clear
          while(analogRead(sensorpin)>650){}
          blocked=0;

    }

// Update errors
    prevErr=err;
    err=readError();
    intErr+=err;
    diffErr=err-prevErr;

// calculate pid value
    pidVal = int(Kp*err +Kd*diffErr +Ki*intErr);

// ensure value within allowable range
    if( pidVal>  speed )  pidVal =  speed; 
    if( pidVal< -speed )  pidVal = -speed; 

// turn by subtracting from wheel speed
    if(pidVal>0){
      analogWrite(motor_right_PWM, speed*offsetR);
      analogWrite(motor_left_PWM, (speed - abs(pidVal))*offsetL);  
    }
    else{
      analogWrite(motor_right_PWM, (speed - abs(pidVal)) *offsetR );
      analogWrite(motor_left_PWM, speed*offsetL);
    } 

    

    
}

float readError(){

// temp variable   
   float tErr=0;
   
// Read and set sensor values to 0 ar 1 
   for (int i=0; i<5; i++){
      int tmp=analogRead(AnalogPin[i]);
      if(tmp > thres) a[i]=0; 
      else a[i]=1;   
    }

// deduce error      
    if(a[0]==0 && a[1]==0 && a[2]==0 && a[3]==0 && a[4]==1) // 00001
        {tErr=4; hardTurn = true;}
     else
    if(a[0]==0 && a[1]==0 && a[2]==0 && a[3]==1 && a[4]==1) // 00011
        {tErr=3; hardTurn = true;}
     else
    if(a[0]==0 && a[1]==0 && a[2]==0 && a[3]==1 && a[4]==0) // 00010
        {tErr=2; hardTurn = true;}
     else
    if(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==1 && a[4]==0) // 00110
        {tErr=1; hardTurn = false;}
     else
    if(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0) // 00100
        {tErr=0; hardTurn = false;}
     else 
    if(a[0]==0 && a[1]==1 && a[2]==1 && a[3]==0 && a[4]==0) // 01100
        {tErr=-1; hardTurn = false;}
     else
    if(a[0]==0 && a[1]==1 && a[2]==0 && a[3]==0 && a[4]==0) // 01000
        {tErr=-2; hardTurn = true;}
     else
    if(a[0]==1 && a[1]==1 && a[2]==0 && a[3]==0 && a[4]==0) // 11000
        {tErr=-3; hardTurn = true;}
     else
    if(a[0]==1 && a[1]==0 && a[2]==0 && a[3]==0 && a[4]==0) // 10000
        {tErr=-4; hardTurn = true;}
     else
     // crossing line
    if(a[0]==1 && a[1]==1 && a[2]==1 && a[3]==1 && a[4]==1) // 11111
        {tErr=0; hardTurn = false;}
     else
    if(a[0]==0 && a[1]==0 && a[2]==0 && a[3]==0 && a[4]==0){
       if(hardTurn==true)
          tErr=5*prevErr/abs(prevErr); // max error when deviated from line   
       else
          tErr=5; // used when rotating
    }
     else
    if(hardTurn==true)
        tErr= 5*prevErr/abs(prevErr); // max error when deviated from line
          
    return tErr;
}

// Motor control methods
void forward(){
     digitalWrite(motor_left_phase, HIGH);
     digitalWrite(motor_right_phase, LOW);
     analogWrite(motor_left_PWM, speed);
     analogWrite(motor_right_PWM, speed);
}

void backward(){
     digitalWrite(motor_left_phase, LOW);
     digitalWrite(motor_right_phase, HIGH);
     analogWrite(motor_left_PWM, speed);
     analogWrite(motor_right_PWM, speed);
}

void ccw(){
     digitalWrite(motor_left_phase, HIGH);
     digitalWrite(motor_right_phase, HIGH);
     analogWrite(motor_left_PWM, speed);
     analogWrite(motor_right_PWM, speed);
}

void cw(){
     digitalWrite(motor_left_phase, LOW);
     digitalWrite(motor_right_phase, LOW);
     analogWrite(motor_left_PWM, speed);
     analogWrite(motor_right_PWM, speed);
}

void stopRobot(){
     digitalWrite(motor_left_PWM, LOW);
     digitalWrite(motor_right_PWM, LOW);
}

void nextpos(int currentpos, int nxtpos){
 
  if(currentpos == 0) zeroFunc(nxtpos);
  
  else if(currentpos == 1){ oneFunc(nxtpos); }
    
  else if(currentpos == 2){ twoFunc(nxtpos);  }
    
  else if(currentpos == 3){ threeFunc(nxtpos);  }
  
  else if(currentpos == 4){ fourFunc(nxtpos); }
  
  else if(currentpos == 5){   
    // yet to be implemented 
  }

  stopRobot();
  
// updatae global variables before returning
  destination=true;
  pos = nextPos;
  miDelay(500);
  
}

// rotation function
//      ahead      -> used to reverse back over line if needed. ie. will be behind the line at the start so no reverse
//      north =  1 -> robot facing 5
//      north = -1 -> robot facing away from 5/south
void chkRot(int set){   
   /* 
    * turn only if facing in direction opposite 
    * to that of the desired direction
   */
   if(set*north==-1){
        readError();
        backward(); while(!(a[0]==1 && a[1]==1 && a[2]==1 && a[3]==1 && a[4]==1)){ backward(); readError();}   // drive back to the line if ahead of line
        stopRobot();
        ccw(); miDelay(350);
        while(readError() ==5 ){}      
        stopRobot();
    } 
  
}

// implements delay without using hardware delay
void miDelay(unsigned long dLay){
    unsigned long currentMs = millis();
    unsigned long prevms = currentMs;
    while(currentMs < prevms + dLay) currentMs = millis(); 
}

void moveToNextpoint(){ 
    forward(); 
    readError();
    while(!(a[0]==1 && a[1]==1 && a[2]==1 && a[3]==1 && a[4]==1)) PID();
    // can update location to server here
}

// coded for junction beside 1
void moveToJunc_CCW_turn(){
   forward(); 
   while(!(a[0]==1 && a[1]==1 && a[2]==1 && a[3]==0 && a[4]==0)){ PID(); }// brings to junction
   miDelay(200);
   stopRobot();
   ccw();
   analogWrite(motor_left_PWM, 0.625*speed);
   analogWrite(motor_right_PWM, 0.625*speed);
   miDelay(250);
   while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();}
}

// coded for junction beside 1-> not tested
void moveToJunc_CW_turn(){
   forward(); 
   while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==1 && a[4]==1)) PID(); // brings to junction
   miDelay(200);
   stopRobot();
   cw();
   analogWrite(motor_left_PWM, 0.625*speed);
   analogWrite(motor_right_PWM, 0.625*speed);
   miDelay(250);
   while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();}
   forward();
}

void passOverLine(){
  miDelay(200);  readError(); // ensures line passed -> update sensor values   
}
 
void zeroFunc(int nxtpos){

/*
 * start used in chkRot as from start position will be behind line.
 * otherwise driving to the line we can usually assume sensors will be 
 * beyond line.
 */
    if(nxtpos == 0){}
    else 
    if(nxtpos == 2){  chkRot(-1);        // want facing south 
                      moveToNextpoint(); // brings us to 2
                      north=1;           // update direction
                    
    }
    else 
    if(nxtpos == 3){  chkRot(-1);        // want facing south
                      moveToNextpoint(); // brings us to 2
                      passOverLine();  
                      moveToNextpoint(); // brings us to 3
                      north=1;
    }
    else
    // this is a bit flakey with the increased speed = 185
    if(nxtpos == 1){  chkRot(-1);           // want facing south
                      moveToJunc_CCW_turn();
                      moveToNextpoint();    // brings us to 1   
                      north=1;              // update direction    
    }
     else
    if(nxtpos == 4){  
                      chkRot(1);         // want facing north
                      moveToNextpoint(); // brings us to 4   
                      north=1;           // update direction           
    }
    else
    if(nxtpos == 5){  chkRot(-1);           // want facing south
                      moveToJunc_CCW_turn();
                      Kp = 15;
                      moveToNextpoint();    // brings us to 1   
                      north=1;              // update direction   
                      Towall();             // Towards the wall
                      stopRobot();   
    }
   
}

void oneFunc(int nxtpos){
    
    if(nxtpos == 1){}
    else 
    if(nxtpos == 2){ chkRot(-1);
                     moveToNextpoint(); // brings us to junction
                     miDelay(150);
                     stopRobot();       //turn at junction
                     ccw();
                     analogWrite(motor_left_PWM, 0.625*speed);
                     analogWrite(motor_right_PWM, 0.625*speed);
                     miDelay(100);
                     while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();} 
                     moveToNextpoint(); // brings us to 2
                     north=1;
    }
    else
    if(nxtpos == 0){ chkRot(-1);
                     moveToNextpoint(); // brings us to junction
                     miDelay(150);
                     stopRobot();
                     cw();
                     analogWrite(motor_left_PWM, 0.625*speed);
                     analogWrite(motor_right_PWM, 0.625*speed);
                     miDelay(100);
                     while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();} 
                     moveToNextpoint(); // brings us to 0 
                     north=1;              
    }
    else
    if(nxtpos == 4){ if(north==-1){
                         backward();
                         miDelay(300);
                         cw();
                         analogWrite(motor_left_PWM, 0.625*speed);
                         analogWrite(motor_right_PWM, 0.625*speed);
                         miDelay(200);
                         readError();
                         while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){readError();}
                         stopRobot();
                         miDelay(50);
                         moveToNextpoint(); 
                         north=-1;
                    }  
                    else if(north == 1){
                        moveToNextpoint();
                        miDelay(100);
                        ccw();
                        analogWrite(motor_left_PWM, 0.625*speed);
                        analogWrite(motor_right_PWM, 0.625*speed);
                        miDelay(200);
                        readError();
                        while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){readError();}
                        stopRobot();
                        miDelay(50);
                        moveToNextpoint(); 
                        north=-1;
                    }            
    }
    else
    if(nxtpos == 3){ if(north==-1){
                         backward();
                         miDelay(300);
                         ccw();
                         analogWrite(motor_left_PWM, 0.625*speed);
                         analogWrite(motor_right_PWM, 0.625*speed);
                         miDelay(200);
                         readError();
                         while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){readError();}
                         stopRobot();
                         miDelay(50);
                         moveToNextpoint(); 
                         north=-1;
                    } 
                    else if(north == 1){
                        moveToNextpoint();
                        miDelay(100);
                        cw();
                        analogWrite(motor_left_PWM, 0.625*speed);
                        analogWrite(motor_right_PWM, 0.625*speed);
                        miDelay(200);
                        readError();
                        while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){readError();}
                        stopRobot();
                        miDelay(50);
                        moveToNextpoint(); 
                        north=-1;
                    }                    
                    
    }   
    else
    if(nxtpos == 5){  chkRot(1);
                      ccw();
                      miDelay(70);  // this is probably very dependent on the speed=210
                      stopRobot();
                      forward();
                      miDelay(150);
                      stopRobot();
                      //readError();
                      //PID();
                      //stopRobot();
                     Towall();         // Towards the wall
    }           
}

void twoFunc(int nxtpos){
    
    if(nxtpos == 1){       // sensors don't really allign at the junction
                      chkRot(-1);
                      moveToJunc_CW_turn();
                      moveToNextpoint();    // brings us to 1   
                      north=1;              // update direction  
    }
    else 
    if(nxtpos == 2){}
    else
    if(nxtpos == 0){
                      chkRot(-1);
                      moveToNextpoint();    // brings us to 0 
                      north=1;              // update direction  
    }
    else
    if(nxtpos == 4){ 
                      chkRot(-1);
                      moveToNextpoint(); // brings us to 0
                      passOverLine();    // updating error
                      moveToNextpoint(); // brings us to 4
                      north=1;           // update direction  
    }
    else
    if(nxtpos == 3){
                      chkRot(1);
                      moveToNextpoint(); // brings us to 3
                      north=1;           // update direction  
    }  
    else
    if(nxtpos == 5){  chkRot(-1);
                      moveToJunc_CW_turn();
                      miDelay(50);
                      Kp = 15;
                      moveToNextpoint();    // brings us to 1   
                      north=1;              // update direction 
                      Towall();             // Towards the wall
    }          
}
   
void threeFunc(int nxtpos){
    
    if(nxtpos == 1){    // have to be tested
                      chkRot(1);
                      forward();
                      while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==1 && a[4]==1)) PID(); // brings to junction
                      analogWrite(motor_left_PWM, speed);
                      analogWrite(motor_right_PWM, speed); 
                      miDelay(250);  
                      ccw();
                      analogWrite(motor_left_PWM, 0.625*speed);
                      analogWrite(motor_right_PWM, 0.625*speed);
                      miDelay(100);
                      while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();}
                      moveToNextpoint();   
                      stopRobot();
                       north=-1;   
    }
    else 
    if(nxtpos == 2){
                      chkRot(-1);
                      moveToNextpoint(); // brings us to 2
                      north=-1;          // update direction  
    }
    else
    if(nxtpos == 0){
                      chkRot(-1);
                      moveToNextpoint(); // brings us to 2
                      passOverLine();    // updating error
                      moveToNextpoint(); // brings us to 0
                      north=1;
    }
    else
    if(nxtpos == 4){    // have to be tested
                      chkRot(1);
                      moveToNextpoint(); // brings us to junction
                      passOverLine();    // updating error
                      moveToNextpoint(); // brings us to 4
                      north=-1;
    }
    else
    if(nxtpos == 3){}    
    else 
    if(nxtpos == 5){
                      chkRot(-1);
                      moveToNextpoint(); // brings us to junction
                      moveToJunc_CW_turn();
                      moveToNextpoint();    // brings us to 1   
//                      north=1;              // update direction  
//                      miDelay(200);
//                      stopRobot();
//                      ccw();
//                      miDelay(100);
//                      while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();}
//                      readError();        // updating error
                      moveToNextpoint(); // brings us to 1
//                      north=-1;
//                      chkRot(1);
                      Towall();          // Towards the wall
    }          
}
   

void fourFunc(int nxtpos){

  if(nxtpos == 0){
                      chkRot(-1);
                      moveToNextpoint();      // brings us to 0
                      north=-1;               // update direction
  }
  else
  if(nxtpos == 1){  // have to be tested
                      chkRot(1);
                      forward();
                      while(!(a[0]==1 && a[1]==1 && a[2]==1 && a[3]==0 && a[4]==0)) PID(); // brings to junction
                      analogWrite(motor_left_PWM, speed);
                      analogWrite(motor_right_PWM, speed); 
                      miDelay(250);  
                      cw();
                      analogWrite(motor_left_PWM, 0.625*speed);
                      analogWrite(motor_right_PWM, 0.625*speed);
                      miDelay(100);
                      while(!(a[0]==0 && a[1]==0 && a[2]==1 && a[3]==0 && a[4]==0)){ readError();}
                      moveToNextpoint();   
                      stopRobot();
                      north=-1;
                      // starts south ends south
  }
  else
  if(nxtpos == 2){
                      chkRot(-1);
                      moveToNextpoint();      // brings us to 0
                      passOverLine();         // updating error
                      moveToNextpoint();      // brings us to 2
                      north=1;
  }
  else
  if(nxtpos == 3){
                      chkRot(1);
//                      moveToNextpoint();      // brings us to the junctions
//                      passOverLine();         // updating error
                      moveToNextpoint();      // brings us to 3
                      north=-1;
  }
  else
  if(nxtpos == 4){}
  else
  if(nxtpos == 5){
                    chkRot(-1);
                      moveToNextpoint(); // brings us to junction
                      passOverLine();
                      moveToJunc_CCW_turn();
                      moveToNextpoint();    // brings us to 1   
                      moveToNextpoint(); // brings us to 1
                      Towall();          // Towards the wall
  }
}

void Towall(){
    analogWrite(motor_left_PWM, speed*offsetL);
    analogWrite(motor_right_PWM, speed*offsetR);
    while(analogRead(sensorpin)<100){ miDelay(40);}
   
    analogWrite(motor_left_PWM, 0.75*speed*offsetL);
    analogWrite(motor_right_PWM, 0.75*speed*offsetR);
    while(analogRead(sensorpin)<250){ miDelay(40);}

    analogWrite(motor_left_PWM, 0.5*speed*offsetL);
    analogWrite(motor_right_PWM, 0.5*speed*offsetR);
    while(analogRead(sensorpin)<500){ miDelay(40);}
    
    analogWrite(motor_left_PWM, 0.15*speed*offsetL);
    analogWrite(motor_right_PWM, 0.15*speed*offsetR);
    while(analogRead(sensorpin)<875){ miDelay(40);}

    stopRobot();
}
