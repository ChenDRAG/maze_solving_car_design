#include <Servo.h>
#include <SR04.h>
#define FRONTD 99 
#define RIGHTD 18
#define LEFTD 180 
int degree(int d)
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
float measure(int d,Servo servo,SR04 sr04)
{
  servo.write(degree(d));
  delay(500);
  return(sr04.DistanceAvg());
}

