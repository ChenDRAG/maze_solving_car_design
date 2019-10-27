# -*- coding: UTF-8 -*-
from multiprocessing import Pool
from urllib.parse import urlencode
import requests
from requests.exceptions import RequestException
import re
import json
from bs4 import BeautifulSoup
import os
pic_pattern=re.compile('BASE_DATA.galleryInfo = {.*?gallery: JSON.parse\("(.*?)"\),',re.S)
def getone(offect,keyword='小狗'):
    dict={
        'offset':str(offect*20),
        'format':'json',
        'keyword':keyword,
        'autoload':'true',
        'count':'20',
        'cur_tab':'3',
        'from':'gallery'
    }
    url='https://www.toutiao.com/search_content/?'+urlencode(dict)
    try:
        response=requests.get(url)
        return(response.text)
    except RequestException:        
        print('请求索引页未知错误')
        return('请求索引页未知错误')
def parseone(html):
    jsons=json.loads(html)
    if jsons and 'data' in jsons.keys():
        for data in jsons['data']:
            if 'article_url' in data.keys():
                change=re.sub('http://toutiao.com/group/','https://www.toutiao.com/a',data['article_url'])
                yield change

def saveone(url):
    def getdetailone(url):
        try:
            response=requests.get(url)
            return(response.text)
        except RequestException:        
            print('请求详情页未知错误')
            return('请求详情页未知错误')    
    def parsedetailone(html):
        piclist=[]
        soup=BeautifulSoup(html,'html.parser')
        tit=soup.title.string
        json_=re.search(pic_pattern,html)    
        tem=json_.group(1)       
        tem=re.sub(r'\\"','\"',tem)
        tem=re.sub(r'\\\\/','/',tem)
        tem=eval("'"+tem+"'")
        json_=json.loads(tem)
        json_=json_["sub_images"]
        for dict in json_:
            piclist.append(dict["url"])
        return tit,piclist 
    print(url)
    try:
        tit,piclist=parsedetailone(getdetailone(url))
    except AttributeError:
        print('不合规范的网页')
        return None
    print(tit)
    tit=r"D:\资料\寒假实践\python库\toutiaojiepai"+'\\'+tit
    try:
        os.mkdir(tit)        
        for i,addr in enumerate(piclist):
            try:
                response=requests.get(addr)
                flow=response.content
                name=tit+'\\'+str(i+1)+".jpeg"
                with open(name,'wb') as f:
                    f.write(flow)
                    f.close()
            except RequestException:        
                print('请求图片未知错误')                            
                
    except OSError:
        print("existed!!!")
def main(offect):  
    with open("keyword.txt",'r') as f:
        input_=f.read()
    html=getone(offect,keyword=input_)   
    gen=parseone(html)
    for url in gen:
        saveone(url)  
        
if __name__=='__main__':      
    with open("keyword.txt",'w') as f:
        f.write(input("keyword?"))
    pool_=Pool()
    groups=[x for x in range(int(input('pages(*20):')))]
    pool_.map(main,groups)            
                
        
    
        
        