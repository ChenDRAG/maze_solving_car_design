#include <Servo.h>
#include <SR04.h>
#include <Motor.h>
#define LQ 3l  //in3
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
#define ENDI  2
#define ENDJ  1
#define FROMI  0
#define FROMJ  4
#define FROMD  W
typedef struct MAP
{
  int times;    //到过该处的次数
  int tendency;
  int *barrier[4];//墙  1可过 0不行 -1未探测//ESWN
} MAP;
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
int ci=FROMI;
int cj=FROMJ;
int cd=FROMD;
int cli=FROMI;
int clj=FROMJ;
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
    floods(ENDI,ENDJ);
  }
  //**************************选择路径
  int sel;
  sel=select();      //输出前后左右
  Serial.println(sel);
  //*************************动
  switch(sel)
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
      cd=(cd+Z)%4;
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
      cd=(cd+Y)%4;
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
      cd=(cd+H)%4;
      delay(100);  
      break;   
  }
  track(); 
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
}
//**************************************************init
void initialiseMap()
{
  for(int i=0;i<5;i++)         //初始化
  {
    for(int j=0;j<5;j++)
    {
      Map[i][j].times=0;
      Map[i][j].tendency=-1;
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
  ci=FROMI;
  cj=FROMJ;
  cd=FROMD; 
}
//*****************************************communicate
void tell()
{
  printMap();
  printtendency();
  Serial.println("wait    wait");
}
void tellandwait()
{
  Serial.println("car now:");
  printcar();
  printMap();
  printtendency();
  printtimes();
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
void printtendency()
{
  for(int i=4;i>=0;i--)
  {
    for(int j=0;j<5;j++)
    {
      Serial.print(Map[i][j].tendency);
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
//************************************************************floods
void floods(int endi,int endj)               //终点坐标
{
  for(int i=0;i<5;i++)
  {
    for(int j=0;j<5;j++)
    {
      Map[i][j].tendency=-1;
    }
  }
  Map[endi][endj].tendency=0;
  flood(0);
}
void flood(int lengthk)  //终点坐标 对lengthk操作
{
  int sym=0;
  for(int i=0;i<5;i++)
  {
    for(int j=0;j<5;j++)
    {
      if(Map[i][j].tendency==lengthk)
      {
        influence(i,j);
        sym=1;  
      }
    }
  }
  if(sym==1)
    flood(lengthk+1);
}
void influence(int fromi,int fromj)  //对fromi,fromj格点操作
{
  int next=Map[fromi][fromj].tendency+1;
  int tem=constrain(fromi+1,0,4);
  if(Map[tem][fromj].tendency==-1&&*(Map[fromi][fromj].barrier[N])!=0)
    Map[tem][fromj].tendency=next;
  tem=constrain(fromi-1,0,4);
  if(Map[tem][fromj].tendency==-1&&*(Map[fromi][fromj].barrier[S])!=0)
    Map[tem][fromj].tendency=next;
  tem=constrain(fromj+1,0,4);
  if(Map[fromi][tem].tendency==-1&&*(Map[fromi][fromj].barrier[E])!=0)
    Map[fromi][tem].tendency=next;  
  tem=constrain(fromj-1,0,4);
  if(Map[fromi][tem].tendency==-1&&*(Map[fromi][fromj].barrier[W])!=0)
    Map[fromi][tem].tendency=next;  
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
  float r,l,f;
  if(*(Map[ci][cj].barrier[cd+Q])==-1)
  {
    f=measurefront(servo,sr04);
    *(Map[ci][cj].barrier[cd+Q])=(f>0.1&&f<17)?0:1;
  }
  if(*(Map[ci][cj].barrier[(cd+Z)%4])==-1)
  {
    l=measureleft(servo,sr04);
    *(Map[ci][cj].barrier[(cd+Z)%4])=(l>3&&l<25)?0:1;
  } 
  if(*(Map[ci][cj].barrier[(cd+Y)%4])==-1)
  {
    r=measureright(servo,sr04);
    *(Map[ci][cj].barrier[(cd+Y)%4])=(r>3&&r<25)?0:1;
  }
}
int select()   
{
  int a[4]={0,0,0,0};
  if(Map[ci][cj].tendency==Map[constrain(ci+1,0,4)][cj].tendency+1&&*(Map[ci][cj].barrier[N])==1)
    a[N]+=10;
  if(Map[ci][cj].tendency==Map[constrain(ci-1,0,4)][cj].tendency+1&&*(Map[ci][cj].barrier[S])==1)
    a[S]+=10;
  if(Map[ci][cj].tendency==Map[ci][constrain(cj+1,0,4)].tendency+1&&*(Map[ci][cj].barrier[E])==1)
    a[E]+=10; 
  if(Map[ci][cj].tendency==Map[ci][constrain(cj-1,0,4)].tendency+1&&*(Map[ci][cj].barrier[W])==1)
    a[W]+=10;  
  a[cd]+=2;
  a[(cd+H)%4]-=2;
  int tem=0;
  Serial.println(a[0]);
  Serial.println(a[1]);
  Serial.println(a[2]);
  Serial.println(a[3]);
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
  

