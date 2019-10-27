#include <Arduino.h>
int serialreadnum()
{
  int i=0,j,k=0;
  int num=0;
  char t;
  int shuzi[6];
  while(1)
  {
    dealy(100);
    t=Serial.read();
    if (t==10)
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
