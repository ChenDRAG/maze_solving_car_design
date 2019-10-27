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
#define E 0
#define S 1
#define W 2
#define N 3
#define Z 3
#define Y 1
#define Q 0
#define H 2
#define FRONTD 99 
#define RIGHTD 11
#define LEFTD 180
typedef struct MAP
{
  boolean times;    //是否到过 
  char tendency;
  char *barrier[4];//墙  1可过 0不行 -1未探测//ESWN
} MAP;
MAP ***Map;
char fromi=0;
char fromj=0;
char fromd=N;
char endi=1;
char endj=0;
char ci;
char cj;
char cd=fromd;
char maphang;
char maplie;
char prettyi;
char prettyj;
unsigned long time=0;
char l,r,t;
Servo servo;
SR04 sr04=SR04(ECHO_PIN,TRIG_PIN);
Motor L=Motor(LQ,LH);
Motor R=Motor(RQ,RH);
void setup()
{
  Serial.begin(9600);
  initinput();  
  initialise();
  servo.attach(POTPIN);
  pinMode(TURN_PIN,INPUT);
  pinMode(RMONITER,INPUT);
  pinMode(LMONITER,INPUT);
  pinMode(LEDPIN,OUTPUT);
  servo.write(degree(90));
  Serial.println("Init Normal!!");
  blink(3);
  Serial.println("GO!");
}


void loop()
{
  //************************扫描探测
  if((*(Map[ci][cj])).times==0)
  {
    (*(Map[ci][cj])).times=1;
    detect();
    floods();
  }
  //**************************选择路径
  char sel;
  sel=select();      //输出前后左右
  Serial.println("making the decision based on the data below:");
  tell();
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
  flash();
  //***************************处理并准备下一次
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
  if((*(Map[ci][cj])).tendency==0)
  {
    Serial.println("SUCCESS!!!");
    while(1)
    {
      digitalWrite(LEDPIN,HIGH);
      delay(3000);
    }
  }
  expand();
  if(Serial.available())
    interrupt();
}
//**************************************************init
void initialise()
{
  initcreate();
  for(char i=0;i<maphang;i++)         //初始化
  {
    for(char j=0;j<maplie;j++)
    {
      (*(Map[i][j])).times=0;
      (*(Map[i][j])).tendency=-1;     
      (*(Map[i][j])).barrier[N]=(char *)malloc(sizeof(char));
      (*(*(Map[i][j])).barrier[N])=-1;
      if(i==0)
      {
        (*(Map[i][j])).barrier[S]=(char *)malloc(sizeof(char));
        (*(*(Map[i][j])).barrier[S])=-1;
      }
      else
        (*(Map[i][j])).barrier[S]=(*(Map[i-1][j])).barrier[N];
      if(j==0)
      {
        (*(Map[i][j])).barrier[W]=(char *)malloc(sizeof(char));
        (*(*(Map[i][j])).barrier[W])=-1;
      }
      else
        (*(Map[i][j])).barrier[W]=(*(Map[i][j-1])).barrier[E];
        (*(Map[i][j])).barrier[E]=(char *)malloc(sizeof(char));
        (*(*(Map[i][j])).barrier[E])=-1;        
    } 
  }
  ci=fromi-prettyi;
  cj=fromj-prettyj;
  cd=fromd;
  endi=endi-prettyi;
  endj=endj-prettyj;
}
void initcreate()
{
  maphang=abs(fromi-endi)+3;
  maplie=abs(fromj-endj)+3;
  Map=(MAP ***)malloc(sizeof(MAP **)*maphang);
	for (char i = 0; i < maphang; i++)
	{
		Map[i]=(MAP **)malloc(sizeof(MAP *)*maplie);
	}
	for (char i = 0; i < maphang; i++)
	{
		for (char j = 0; j < maplie; j++)
		{
			Map[i][j]=(MAP *)malloc(sizeof(MAP));
		}
	}
  prettyi=(fromi>endi)?(endi-1):(fromi-1);
  prettyj=(fromj>endj)?(endi-1):(fromi-1);
}
//***************************************************mapsort
void expand()
{
  if(ci==maphang-1)
  {
    Serial.println("in1  ci==maphang-1");
    MAP ***tem=(MAP ***)malloc(sizeof(MAP **)*(maphang+1));
    for (char i = 0; i < maphang; i++)
      tem[i]=Map[i];
    tem[maphang]=(MAP **)malloc(sizeof(MAP *)*maplie);
    for (char j = 0; j < maplie; j++)
    {
       tem[maphang][j]=(MAP *)malloc(sizeof(MAP));
      (*(tem[maphang][j])).tendency=-1;
      (*(tem[maphang][j])).times=0;
      (*(tem[maphang][j])).barrier[S]=(*(tem[maphang-1][j])).barrier[N];
      (*(tem[maphang][j])).barrier[N]=(char *)malloc(sizeof(char));
      (*(*(tem[maphang][j])).barrier[N])=-1;
      (*(tem[maphang][j])).barrier[E]=(char *)malloc(sizeof(char));
      (*(*(tem[maphang][j])).barrier[E])=-1; 
      if(j==0)
      {
        (*(tem[maphang][j])).barrier[W]=(char *)malloc(sizeof(char));
        (*(*(tem[maphang][j])).barrier[W])=-1;
      }
      else
       (*(tem[maphang][j])).barrier[W]=(*(tem[maphang][j-1])).barrier[E];     
    }    
    Serial.println("in2");
    free(Map);
    Serial.println("in3");
    Map=tem; 
    maphang++;
    Serial.println("in4");
  }
  if(ci==0)
  {
    MAP ***tem=(MAP ***)malloc(sizeof(MAP **)*(maphang+1));
    for (char i = 0; i < maphang; i++)
      tem[i+1]=Map[i];
    tem[0]=(MAP **)malloc(sizeof(MAP *)*maplie);
    for (char j = 0; j < maplie; j++)
    {
      tem[0][j]=(MAP *)malloc(sizeof(MAP));
      (*(tem[0][j])).tendency=-1;
      (*(tem[0][j])).times=0;
      (*(tem[0][j])).barrier[N]=(*(tem[1][j])).barrier[S];
      (*(tem[0][j])).barrier[S]=(char *)malloc(sizeof(char));
      (*(*(tem[0][j])).barrier[S])=-1;
      (*(tem[0][j])).barrier[E]=(char *)malloc(sizeof(char));
      (*(*(tem[0][j])).barrier[E])=-1; 
      if(j==0)
      {
        (*(tem[0][j])).barrier[W]=(char *)malloc(sizeof(char));
        (*(*(tem[0][j])).barrier[W])=-1;
      }
      else
       (*(tem[0][j])).barrier[W]=(*(tem[0][j-1])).barrier[E];     
     }    
    free(Map);
    Map=tem; 
    maphang++;
    prettyi--;
    endi++;
    ci++;
  }
  if(cj==maplie-1)
  {
    Serial.println("cj==maplie-1");
    for(char i=0;i<maphang;i++)
    {
      MAP **tem=(MAP **)malloc(sizeof(MAP *)*(maplie+1));
      if(tem==0) Serial.println("fail!");
      for(char j=0;j<maplie;j++)
      {
        tem[j]=Map[i][j];
      }
      tem[maplie]=(MAP *)malloc(sizeof(MAP));
      (*(tem[maplie])).times=0;
      (*(tem[maplie])).tendency=-1;
      free(Map[i]);
      Map[i]=tem;      
    }
    Serial.println("in1");
    for(char i=0;i<maphang;i++)
    {
      (*(Map[i][maplie])).barrier[W]=(*(Map[i][maplie-1])).barrier[E];
      (*(Map[i][maplie])).barrier[E]=(char *)malloc(sizeof(char));
      (*(*(Map[i][maplie])).barrier[E])=-1;
      (*(Map[i][maplie])).barrier[N]=(char *)malloc(sizeof(char));
      (*(*(Map[i][maplie])).barrier[N])=-1;  
      if(i==0)
      {
        (*(Map[i][maplie])).barrier[S]=(char *)malloc(sizeof(char));
        (*(*(Map[i][maplie])).barrier[S])=-1;     
      }      
      else
        (*(Map[i][maplie])).barrier[S]=(*(Map[i-1][maplie])).barrier[N];
    }
    Serial.println("in2");
    maplie++;
  }
  if(cj==0)
  {
    for(char i=0;i<maphang;i++)
    {
      MAP **tem=(MAP **)malloc(sizeof(MAP *)*(maplie+1));
      for(char j=0;j<maplie;j++)
      {
        tem[j+1]=Map[i][j];
      }
      tem[0]=(MAP *)malloc(sizeof(MAP));
      (*(tem[0])).times=0;
      (*(tem[0])).tendency=-1;
      free(Map[i]);
      Map[i]=tem;      
    }
    for(char i=0;i<maphang;i++)
    {
      (*(Map[i][0])).barrier[E]=(*(Map[i][1])).barrier[W];
      (*(Map[i][0])).barrier[W]=(char *)malloc(sizeof(char));
      (*(*(Map[i][0])).barrier[W])=-1;
      (*(Map[i][0])).barrier[N]=(char *)malloc(sizeof(char));
      (*(*(Map[i][0])).barrier[N])=-1;  
      if(i==0)
      {
        (*(Map[i][0])).barrier[S]=(char *)malloc(sizeof(char));
        (*(*(Map[i][0])).barrier[S])=-1;     
      }      
      else
        (*(Map[i][0])).barrier[S]=(*(Map[i-1][0])).barrier[N];
    }
    maplie++;
    endj++;
    cj++;
    prettyj--;
  }  
  Serial.println("out");
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
void tell()
{
  Serial.print("car now:");
  Serial.print("(");
  Serial.print(prettyi+ci,DEC);
  Serial.print(",");
  Serial.print(prettyj+cj,DEC);
  Serial.println(")");
  Serial.print("your car'd(0E1S2W3N):");
  Serial.println(cd,DEC);
  Serial.print("below your car is at:");
  Serial.print("(");
  Serial.print(ci,DEC);
  Serial.print(",");
  Serial.print(cj,DEC);
  Serial.println(")");
  printMap();
  printtendency();
}
void interrupt()
{
  Serial.read();
  Serial.println("Car stopped i for info,g for go.......");
  while(1)
  {
    if(Serial.available())
    {
      char temp;
      temp=Serial.read();
      if(temp=='i')
        tell();
      if(temp=='g')
      {
        Serial.println("Car go in one second...");
        break;
      }        
    }
  blink(1);
  }
}
void printMap()
{
  Serial.println("Printing the map...");
  for(char i=maphang-1;i>=0;i--)
  {
    for(char j=0;j<maplie;j++)
    {
      Serial.write("##");
      if((*(*(Map[i][j])).barrier[N])==0)
        Serial.write("##");
      else if((*(*(Map[i][j])).barrier[N])==-1)
        Serial.write("??");
      else
        Serial.write("oo");
    }
    Serial.println("##"); 
    for(char j=0;j<maplie;j++)
    {
      if((*(*(Map[i][j])).barrier[W])==0)
        Serial.write("##");
      else if((*(*(Map[i][j])).barrier[W])==-1)
        Serial.write("??");
      else
        Serial.write("oo");
      Serial.write("  ");
    }
    if((*(*(Map[i][maplie-1])).barrier[E])==0)
      Serial.write("##");
    else if((*(*(Map[i][maplie-1])).barrier[E])==-1)
      Serial.write("??");
    else
      Serial.write("oo");   
     Serial.write(10); 
  }
  for(char j=0;j<maplie;j++)
  {
    Serial.write("##");
    if((*(*(Map[0][j])).barrier[S])==0)
      Serial.write("##");
    else if((*(*(Map[0][j])).barrier[S])==-1)
      Serial.write("??");
    else     
      Serial.write("oo");
  }
  Serial.println("##");
  Serial.print("maphang:");
  Serial.println(maphang,DEC);
  Serial.print("maplie:");
  Serial.println(maplie,DEC);  
}
void printtendency()
{
  Serial.println("Printing the tendency...");
  for(char i=maphang-1;i>=0;i--)
  {
    for(char j=0;j<maplie;j++)
    {
      Serial.print((*(Map[i][j])).tendency,DEC);
      Serial.print(',');
    }
    Serial.println(' ');
  }
}
void initinput()
{
  char t;
  Serial.println("Please input the starting and ending point(in the form of (fromi,fromj),(endi,endj)):");
  while(1)
  {
    if(Serial.available())
    {
      delay(100);
      break;
    }
    blink(1);
  }
  t=Serial.read();
  if(t=='('&&Serial.available())
  {
    fromi=serialreadnum(',');
    fromj=serialreadnum(')');
    t=Serial.read();
    t=Serial.read();
    if(t=='('&&Serial.available())
    {
      endi=serialreadnum(',');
      endj=serialreadnum(')');
    }
    else
      goto errorsort;
  }
  else
    goto errorsort;
  Serial.println("Please input the starting direction(0 for E,1 for S,2 for W,3 for N):");
  while(1)
  {
    if(Serial.available())
    {
      delay(100);
      break;
    }
    blink(1);
  } 
  fromd=map(Serial.read(),'0','9',0,9);
  if(Serial.available())
    goto errorsort;
  Serial.print("your car starts at (");
  Serial.print(fromi,DEC);
  Serial.print(",");
  Serial.print(fromj,DEC);
  Serial.println(")");
  Serial.print("present direction:");
  Serial.println(fromd,DEC);
  Serial.print("your car is going to end at (");
  Serial.print(endi,DEC);
  Serial.print(",");
  Serial.print(endj,DEC);
  Serial.println(")");  
  Serial.println("Check if this is right(y for yes,n for no)");
  while(1)
  {
    if(Serial.available())
    {
      delay(100);
      break;
    }
    blink(1);
  } 
  t=Serial.read();
  if(t=='y')
    return;
  else
    goto errorsort;
errorsort:
  {
    Serial.println("some unknown error happened,please do this again");
    initinput();
  }
}

char serialreadnum(char finish)
{
  char i=0,j,k=0;
  char num=0;
  char t;
  char shuzi[6];
  while(1)
  {
    delay(100);
    t=Serial.read();
    if (t==finish)
      break;
    shuzi[i]=map(t,'0','9',0,9);
    i++;
    if(Serial.available()==0)
      break;
  } 
  for(j=i-1;j>=0;j--)
  {
     num+=shuzi[j]*pow(10,k++);
  } 
  return(num);
}
//************************************************************floods
void floods()               //终点坐标
{
  for(char i=0;i<maphang;i++)
  {
    for(char j=0;j<maplie;j++)
    {
      (*(Map[i][j])).tendency=-1;
    }
  }
  (*(Map[endi][endj])).tendency=0;
  flood(0,0);
}
void flood(char lengthk,char endding)  //终点坐标 对lengthk操作
{
  char sym=0;
  for(char i=0;i<maphang;i++)
  {
    for(char j=0;j<maplie;j++)
    {
      if((*(Map[i][j])).tendency==lengthk)
      {
        influence(i,j);
        sym=1;  
      }
    }
  }
  if(endding==1)
    return;
  if((*(Map[ci][cj])).tendency==-1)
    flood(lengthk+1,0);
  else
    flood(lengthk+1,1);
}
void influence(char fromi,char fromj)  //对fromi,fromj格点操作
{
  char next=(*(Map[fromi][fromj])).tendency+1;
  char tem=constrain(fromi+1,0,maphang);
  if((*(Map[tem][fromj])).tendency==-1&&*((*(Map[fromi][fromj])).barrier[N])!=0)
    (*(Map[tem][fromj])).tendency=next;
  tem=constrain(fromi-1,0,maphang);
  if((*(Map[tem][fromj])).tendency==-1&&*((*(Map[fromi][fromj])).barrier[S])!=0)
    (*(Map[tem][fromj])).tendency=next;
  tem=constrain(fromj+1,0,maplie);
  if((*(Map[fromi][tem])).tendency==-1&&*((*(Map[fromi][fromj])).barrier[E])!=0)
    (*(Map[fromi][tem])).tendency=next;  
  tem=constrain(fromj-1,0,maplie);
  if((*(Map[fromi][tem])).tendency==-1&&*((*(Map[fromi][fromj])).barrier[W])!=0)
    (*(Map[fromi][tem])).tendency=next;  
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
//*********************************************************carmain
void detect()
{
  float r,l,f;
  if((*(*(Map[ci][cj])).barrier[cd+Q])==-1)
  {
    f=measurefront(servo,sr04);
    (*(*(Map[ci][cj])).barrier[cd+Q])=(f>0.1&&f<17)?0:1;
  }
  if((*(*(Map[ci][cj])).barrier[(cd+Z)%4])==-1)
  {
    l=measureleft(servo,sr04);
    (*(*(Map[ci][cj])).barrier[(cd+Z)%4])=(l>3&&l<25)?0:1;
  } 
  if((*(*(Map[ci][cj])).barrier[(cd+Y)%4])==-1)
  {
    r=measureright(servo,sr04);
    (*(*(Map[ci][cj])).barrier[(cd+Y)%4])=(r>3&&r<25)?0:1;
  }
}
char select()   
{
  char a[4]={0,0,0,0};
  if(((*(Map[ci][cj])).tendency==(*(Map[constrain(ci+1,0,maphang)][cj])).tendency+1)&&(*(*(Map[ci][cj])).barrier[N])==1)
    a[N]+=10;
  if(((*(Map[ci][cj])).tendency==(*(Map[constrain(ci-1,0,maphang)][cj])).tendency+1)&&(*(*(Map[ci][cj])).barrier[S])==1)
    a[S]+=10;
  if(((*(Map[ci][cj])).tendency==(*(Map[ci][constrain(cj+1,0,maplie)])).tendency+1)&&(*(*(Map[ci][cj])).barrier[E])==1)
    a[E]+=10; 
  if(((*(Map[ci][cj])).tendency==(*(Map[ci][constrain(cj-1,0,maplie)])).tendency+1)&&(*(*(Map[ci][cj])).barrier[W])==1)
    a[W]+=10;  
  a[cd]+=2;
  a[(cd+H)%4]-=2;
  char tem=0;
  Serial.print("the socre of E S W N are:");
  Serial.print(a[E],DEC);
  Serial.print("   ");
  Serial.print(a[S],DEC);
  Serial.print("   "); 
  Serial.print(a[W],DEC);
  Serial.print("   "); 
  Serial.print(a[N],DEC);
  Serial.println("   ");
  for(char k=0;k<4;k++)
  {
    if(a[k]>tem)
      tem=a[k];
  }
  for(char k=0;k<4;k++)
  {
    if(a[k]==tem)
    {
      Serial.print("going: ");
      Serial.println(k,DEC);
      k=abs((k-cd+12)%4);
      return(k);
    }
  }
}

