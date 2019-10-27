

#include <Motor.h>
#define LQ 11
#define LH 10
#define RQ 5
#define RH 6
Motor R=Motor(RQ,RH);
Motor L=Motor(LQ,LH);
const int zhuangtai[6][3]=      //0前进1停止2后退3左转4右转5后转
{
{200,200,0},
{0,0,0},
{-150,-150,0},
{-200,200,1000},
{200,-200,1000},
{200,-200,2000}
};
void motordo(int zt)
{
  R.go(zhuangtai[zt][0]);
  L.go(zhuangtai[zt][1]);
  if(zhuangtai[zt][4]!=0)
  {
    delay(zhuangtai[zt][2]);
    motordo(1);
  }
}
