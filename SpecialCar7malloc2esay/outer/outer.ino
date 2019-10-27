#include <Servo.h>
#include <SR04.h>
#include <Motor.h>
#define LQ 3  //in3
#define LH 11 //in4
#define RQ 5   //in2
#define RH 6   //in1
#define TRIG_PIN 8
#define ECHO_PIN 7
#define POTPIN 9
#define LEDPIN 13
#define TURN_PIN 2
#define RMONITER 4
#define LMONITER 12
#define Z 3
#define Y 1
#define Q 0
#define H 2
#define FRONTD 99 
#define RIGHTD 11
#define LEFTD 180
int l,r,t;
unsigned long time=0;
Servo servo;
SR04 sr04=SR04(ECHO_PIN,TRIG_PIN);
Motor L=Motor(LQ,LH);
Motor R=Motor(RQ,RH);
void setup()
{
  Serial.begin(9600);
  servo.attach(POTPIN);
  pinMode(TURN_PIN,INPUT);
  pinMode(RMONITER,INPUT);
  pinMode(LMONITER,INPUT);
  pinMode(LEDPIN,OUTPUT);
  servo.write(degree(90));
  flash();
  Serial.write('g');
  hear();
  blink(3);
}
void loop()
{
  //************************扫描探测 
 Serial.write('t'); 
 if(hear()=='y')
   detect();
   //*************************动
  switch(hear())
  {
    case Q:
      break;
    case Z:
      L.go(0);
      L.go(-100);
      R.go(100);
      delay(500);
      while(1)
      {
        delay(1);
        if(digitalRead(LMONITER)==0)
        {
          R.go(0);
          L.go(0);
          break;
        }
      }
      delay(100);
      break;
    case Y:
      L.go(100);
      R.go(-100);
      delay(500);
      while(1)
      {
        delay(1);
        if(digitalRead(RMONITER)==0)
        {
          R.go(0);
          L.go(0);
          break;
        }
      }
      delay(100); 
      break;
    case H:
      L.go(0);
      R.go(100);
      L.go(-100);
      delay(1250);
      while(1)
      {
        delay(1);
        if(digitalRead(LMONITER)==0)
        {
          R.go(0);
          L.go(0);
          break;
        }
      }
      delay(100);  
      break;   
  }
  track(); 
  if(hear()=='m')
    while(1)
      digitalWrite(LEDPIN,1);
}

//********************************************************led
void blink(char t)
{  
  long tt=millis();
  while(millis()-tt<t)
  {
    digitalWrite(LEDPIN, HIGH);  
    delay(500);              
    digitalWrite(LEDPIN, LOW);   
    delay(500);  
  }    
}
void flash()
{
  digitalWrite(LEDPIN,1);
  delay(50);
  digitalWrite(LEDPIN,0);  
}
//*****************************************communicate
char hear()
{
  while(1)
    if(Serial.available())
    {
      flash();
       return(Serial.read());
    }
}
void detect()
{
  float r,l,f;
  f=measurefront(servo,sr04);
  f=(f>0.1&&f<17)?0:1;
  Serial.write((int)f);
  l=measureleft(servo,sr04);
  l=(l>3&&l<25)?0:1;
  Serial.write((int)l);
  r=measureright(servo,sr04);
  r=(r>3&&r<25)?0:1;
  Serial.write((int)r);
}
//************************************************************move
void track()
{
  time=millis();
  while(1)
  {
    l=digitalRead(LMONITER);
    r=digitalRead(RMONITER);
    t=digitalRead(TURN_PIN);
    if(r==0&&l==0)
    {
      L.go(200);
      R.go(200);
    }
    else if(r==1&&l==0)
    {
      L.go(100);
      R.go(200);    
    }
    else if(r==0&&l==1)
    {
      L.go(200);
      R.go(100);    
    }
    else
    {
      L.go(0);
      R.go(0);  
      L.go(-200);
      R.go(-200);        
    }
    if(millis()-time>1000&&t==1)
    {
      delay(20);
      if(digitalRead(TURN_PIN)==1)
      {
        L.go(0);
        R.go(0);  
        break;
      }
    }
  }
}
//*****************************************************************(servo)
char degree(char d)
{
  return(map(d,0,180,18,180)); 
}
float measureright(Servo servo,SR04 sr04)
{
  servo.write(RIGHTD);
  delay(500);
  return(sr04.DistanceAvg());
}
float measureleft(Servo servo,SR04 sr04)
{
  servo.write(LEFTD);
  delay(500);
  return(sr04.DistanceAvg());
}
float measurefront(Servo servo,SR04 sr04)
{
  servo.write(FRONTD);
  delay(500);
  return(sr04.DistanceAvg());
}
