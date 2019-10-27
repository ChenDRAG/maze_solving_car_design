#include <Servo.h>
#include <SR04.h>
#include <Motor.h>
#define LQ 11  //in3
#define LH 10  //in4
#define RQ 5   //in2
#define RH 6   //in1
#define TRIG_PIN 8
#define ECHO_PIN 7
#define POTPIN 9
#define LEDPIN 13
#define TURN_PIN 2
#define RMONITER 4
#define LMONITER 12
#define E 0
#define S 1
#define W 2
#define N 3
#define Z 3
#define Y 1
#define Q 0
#define H 2
#define FRONTD 99 
#define RIGHTD 18
#define LEFTD 180 
typedef struct MAP
{
  int times;    //到过该处的次数
  int choices;   //当前在该处可以走的路条数0,1,2,3,4,-1表示未探测
  int tendency;
  int *barrier[4];//墙  1可过 0不行 -1未探测//ESWN
} MAP;
const int zhuangtai[6][3]=      //0前进1停止2后退3左转4右转5后转
{
{200,200,0},
{0,0,0},
{150,150,0},
{-200,200,700},
{200,-200,700},
{200,-200,1300}
};
int Barrier1[6][5]=           //上下
{
{0,0,0,0,0},
{-1,-1,-1,-1,-1},
{-1,-1,-1,-1,-1},
{-1,-1,-1,-1,-1},
{-1,-1,-1,-1,-1},
{0,0,0,0,0}
};
int Barrier2[5][6]=//左右
{
{0,-1,-1,-1,-1,0},
{0,-1,-1,-1,-1,0},
{0,-1,-1,-1,-1,0},
{0,-1,-1,-1,-1,0},
{0,-1,-1,-1,-1,0}
};
int ci=0;
int cj=0;
int cd=0;
int cli=0;
int clj=0;
unsigned long time=0;
int l,r,t;
MAP Map[5][5];
Servo servo;
SR04 sr04=SR04(ECHO_PIN,TRIG_PIN);
Motor L=Motor(LQ,LH);
Motor R=Motor(RQ,RH);
void setup()
{      
  initialiseMap();
  initialiseCar();
  servo.attach(POTPIN);
  pinMode(TURN_PIN,INPUT);
  pinMode(RMONITER,INPUT);
  pinMode(LMONITER,INPUT);
  pinMode(LEDPIN,OUTPUT);
  servo.write(degree(90));
  Serial.begin(9600);
  Serial.println("Normal!!");
  tell();
  blink(3);
}


void loop()
{
  //************************扫描探测
  if(Map[ci][cj].times==1)
  {
    detect();
    Map[ci][cj].choices=*(Map[ci][cj].barrier[0])+*(Map[ci][cj].barrier[1])+*(Map[ci][cj].barrier[2])+*(Map[ci][cj].barrier[3]);
  }
  //**************************选择路径
  tell();
  int sel;
  sel=select();      //输出前后左右
  Serial.println(sel);
  tell();
  //*************************动
  switch(sel)
  {
    case Q:
      delay(1000);
      break;
    case Z:
      L.go(-100);
      R.go(100);
      delay(750);
      L.go(0);
      R.go(0);
      cd=(cd+Z)%4;
      delay(1000);
      break;
    case Y:
      L.go(100);
      R.go(-100);
      delay(750);
      L.go(0);
      R.go(0);
      cd=(cd+Y)%4;
      delay(1000); 
      break;
    case H:
      L.go(-100);
      R.go(100);
      delay(1600);
      L.go(0);
      R.go(0);
      cd=(cd+H)%4;
      delay(1000);  
      break;   
  }
  tell();
  track();
  while(1)
  {
    if(digitalRead(TURN_PIN)==1)
    {
      delay(50);
      if(digitalRead(TURN_PIN)==1)
        break;
    }
    delay(5);
  }  
  while(1)
  {
    if(digitalRead(TURN_PIN)==0)
    {
      delay(50);
      if(digitalRead(TURN_PIN)==0)
      {
        L.go(0);
        R.go(0);
        break;
      }
    }
    delay(5); 
  }
  tell();
  //***************************处理并准备下一次
  cli=ci;
  clj=cj;
  switch(cd)
  {
    case N:
      ci++;
      break;
    case S:
      ci--; 
      break;
    case E:
      cj++;  
      break; 
    case W:
      cj--; 
      break;     
  }
  if(Map[ci][cj].tendency==0)
    while(1)
    {
      digitalWrite(LEDPIN,HIGH);
      delay(3000);
    }
  Map[ci][cj].times++;
  if(Map[cli][clj].times>=Map[cli][clj].choices)
    for(int i=0;i<4;i++)
      *(Map[cli][clj].barrier[i])=0; 
  delay(1000);
  tell();
}
//**************************************************init
void initialiseMap()
{
  int tem[5][5]=
  {
  {4,3,2,3,4},
  {3,2,1,2,3},
  {2,1,0,1,2},
  {3,2,1,2,3},
  {4,3,2,3,4}
  };
  for(int i=0;i<5;i++)         //初始化
  {
    for(int j=0;j<5;j++)
    {
      Map[i][j].times=0;
      Map[i][j].choices=-1;
      Map[i][j].tendency=tem[i][j];
      Map[i][j].barrier[N]=&Barrier1[i+1][j];
      Map[i][j].barrier[S]=&Barrier1[i][j];;
      Map[i][j].barrier[W]=&Barrier2[i][j];
      Map[i][j].barrier[E]=&Barrier2[i][j+1];
    } 
  }
  Map[0][0].times=1;
}
void initialiseCar()
{
  ci=0;
  cj=0;
  cd=N; 
}
//*****************************************communicate
void tell()
{
  Serial.println("car now:");
  printcar();
  printMap();
  printtimes();
  printchoices();
  Serial.println("wait    wait");
  while(1)
  {
    if (Serial.available()) 
    {
      Serial.println(Serial.read());
      delay(3000);
      break;
    }
    delay(1000);
    blink(2000);
  }
}
void printMap()
{
  for(int i=4;i>=0;i--)
  {
    for(int j=0;j<5;j++)
    {
      Serial.write("  ");
      if(*(Map[i][j].barrier[N])==0)
        Serial.write("##");
      else if(*(Map[i][j].barrier[N])==-1)
        Serial.write("??");
      else
        Serial.write("oo");
    }
    Serial.println("  "); 
    for(int j=0;j<5;j++)
    {
      if(*(Map[i][j].barrier[W])==0)
        Serial.write("##");
      else if(*(Map[i][j].barrier[W])==-1)
        Serial.write("??");
      else
        Serial.write("oo");
      Serial.write("  ");
    }
    if(*(Map[i][4].barrier[E])==0)
      Serial.write("##");
    else if(*(Map[i][4].barrier[E])==-1)
      Serial.write("??");
    else
      Serial.write("oo");   
     Serial.write(10); 
  }
  for(int j=0;j<5;j++)
  {
    Serial.write("  ");
    if(*(Map[0][j].barrier[S])==0)
      Serial.write("##");
    else if(*(Map[0][j].barrier[S])==-1)
      Serial.write("??");
    else     
      Serial.write("oo");
  }
  Serial.println("  ");
}
void printchoices()
{
  for(int i=4;i>=0;i--)
  {
    for(int j=0;j<5;j++)
    {
      Serial.print(Map[i][j].choices);
      Serial.print(',');
    }
    Serial.println(' ');
  }
}
void printtimes()
{
  for(int i=4;i>=0;i--)
  {
    for(int j=0;j<5;j++)
    {
      Serial.print(Map[i][j].times);
      Serial.print(',');
    }
    Serial.println(' ');
  }
}
void printcar()
{
  Serial.println("car:");
  Serial.print("(ci,cj):");
  Serial.print("(");
  Serial.print(ci);
  Serial.print(",");
  Serial.print(cj);
  Serial.println(")");
  Serial.print("cd:");
  Serial.println(cd);
  Serial.print("(cli,clj):");
  Serial.print("(");
  Serial.print(cli);
  Serial.print(",");
  Serial.print(clj);
  Serial.println(")");  
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
    if(millis()-time>2000&&t==1)
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
//********************************************************led
void blink(int t)
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
//*********************************************************carmain
void detect()
{
  bool b[4]={1,1,1,1};
  float r,l,f;
  f=measurefront(servo,sr04);
  if(0.1<f&&f<17)
    b[Q]=0;
  r=measureright(servo,sr04);
  if(3<r&&r<25)
    b[Y]=0;
  l=measureleft(servo,sr04);
  if(3<l&&l<25)
    b[Z]=0;
  if(*(Map[ci][cj].barrier[cd+Q])==-1)
    *(Map[ci][cj].barrier[cd+Q])=b[Q];
  if(*(Map[ci][cj].barrier[(cd+Z)%4])==-1)
    *(Map[ci][cj].barrier[(cd+Z)%4])=b[Z];
  if(*(Map[ci][cj].barrier[(cd+Y)%4])==-1)
    *(Map[ci][cj].barrier[(cd+Y)%4])=b[Y];
}
int select()   
{
  int a[4]={0,0,0,0};
  for(int i=0;i<4;i++)
  {
    a[i]+=*(Map[ci][cj].barrier[i])*100;
  }
  if(Map[ci][cj].tendency>Map[constrain(ci+1,0,4)][cj].tendency)
    a[N]+=10;
  if(a[0]>=100&&Map[ci][cj].tendency>Map[constrain(ci-1,0,4)][cj].tendency)
    a[S]+=10;
  if(a[2]>=100&&Map[ci][cj].tendency>Map[ci][constrain(cj+1,0,4)].tendency)
    a[E]+=10; 
  if(a[3]>=100&&Map[ci][cj].tendency>Map[ci][constrain(cj-1,0,4)].tendency)
    a[W]+=10;  
  a[cd]+=2;
  a[(cd+H)%4]-=50;
  int tem=0;
  for(int k=0;k<4;k++)
  {
    if(a[k]>tem)
      tem=a[k];
  }
  for(int k=0;k<4;k++)
  {
    if(a[k]==tem)
    {
      k=abs((k-cd+12)%4);
      return(k);
    }
  }
}
  

