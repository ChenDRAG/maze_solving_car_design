# -*- coding: UTF-8 -*-
import requests
from requests.exceptions import RequestException
import re
pattern_findinfo=re.compile('<dd>.*?<a href="(.*?)" title="(.*?)".*?<img data-src="(.*?)".*?'
+'(<p class="star">(.*?)</p>.*?)?(<p class="releasetime">(.*?)</p>.*?)?'
+'(<p class="score"><i class="integer">(.*?)</i><i class="fraction">(.*?)</i>.*?)?'
+'</dd>',re.S)
pattern_finddes=re.compile('<div class="mod-content">.*?<span class="dra">(.*?)</span>.*?</div>',re.S)
pattern_findnextpageaddr=re.compile('<a.*?\s*?.*?href="(.*?)"\s*?>下一页</a>')
infos=[]
url=''
def getone(url):
    try:
        response=requests.get(url)
        return(response.text)
    except RequestException:
        return('false')
def selectinfo(html):
    global infos
    items=re.findall(pattern_findinfo,html)
    for item in items:
        tem={}
        url='http://maoyan.com'+item[0]
        tem['name']=item[1]
        tem['score']=item[8]+item[9]
        tem['actors']=item[4].strip()[3:]
        tem['showtime']=item[6][5:]
        tem['description']=desget(url)      
        tem['imageurl']=item[2]
        tem['url']=url
        infos.append(tem)
def desget(url):
    html=getone(url)
    result=re.search(pattern_finddes,html)
    return(result.group(1))
def selectnext(html): 
    addr=re.search(pattern_findnextpageaddr,html)
    if addr!=None:
        addr=url+addr.group(1)
    return(addr)
def creat(name):
    global infos
    name='D:\\资料\\寒假实践\\python库\\maoyantop\\'+name
    name=name+'.txt'
    with open(name,'w',encoding='utf-8') as f:
        for i,info in enumerate(infos):   
            f.write(str(i+1)+'\n') 
            for key,thing in info.items():
                f.write(key+':'+thing+'\n')
            f.write('\n')
    infos=[]
def main(url):        
    while 1: 
        print(url)
        html=getone(url)    
        selectinfo(html)
        next=selectnext(html)
        if next==None:
            break
        else:
            url=next        


url='http://maoyan.com/board/2'
main(url)
creat('northamericatop')
url='http://maoyan.com/board/1'
main(url)
creat('chinatop')
url='http://maoyan.com/board/4'
main(url)
creat('top100')
url='http://maoyan.com/board/6'
main(url)
creat('mostexcepted')
url='http://maoyan.com/board/7'
main(url)
creat('besttop')