#include <Motor.h>
#include "read.h"
#include <Motor.h>
#define LQ 11   //in3
#define LH 10  //in4
#define RQ 5   //in2
#define RH 6   //in1
int d=90;
Motor R=Motor(RQ,RH);
Motor L=Motor(LQ,LH);
const int zhuangtai[6][3]=      //0前进1停止2后退3左转4右转5后转
{
{200,200,0},
{0,0,0},
{-150,-150,0},
{200,-200,500},
{-200,200,500},
{200,-200,1000}
};
void setup()
{      
  Serial.begin(9600);
 
  Serial.println("normal");
}
void loop()
{
  if(Serial.available())
  {
    d=serialreadnum();
    Serial.println(d);
    motordo(d);
  }
}
void motordo(int zt)
{
  R.go(zhuangtai[zt][0]);
  L.go(zhuangtai[zt][1]);
  if(zhuangtai[zt][2]!=0)
  {
    delay(zhuangtai[zt][2]);
    R.go(0);
    L.go(0);
  }
}
