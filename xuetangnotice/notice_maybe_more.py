# -*- coding: UTF-8 -*-
from multiprocessing import Pool
from urllib.parse import urlencode
import requests
from requests.exceptions import RequestException
import re
import json
import os
from selenium import webdriver
import time
pattern_name=re.compile('标题 (.*?)正文',re.S)
pattern_text=re.compile('正文(.*?)$',re.S)
chromedriver = r"C:\Program Files (x86)\Google\Chrome\Application\chromedriver.exe"
os.environ["webdriver.chrome.driver"] = chromedriver
browser = webdriver.Chrome(chromedriver)   
def save_txt(content,name,path=None,renew=1):
    if path!=None:
        path+=(os.path.sep+name+'.txt')
    else:
        path=(name+'.txt')
    if renew!=1:
        if os.path.exists(path):
            return
    with open(path,'w',encoding='utf-8') as f:
        f.write(content)
        f.close
        
def login():
    input=browser.find_element_by_css_selector('input[type="text"][name="userid"]')
    input.send_keys('chenhuay17')
    input=browser.find_element_by_css_selector('input[type="password"][name="userpass"]')
    input.send_keys('QHCHENHUAYUABCDlong')
    button=browser.find_element_by_css_selector('input[type="submit"][value="登录"]')
    button.click()   
    #browser.find_element_by_css_selector('a[href="MyCourse.jsp?typepage=2"]').click()
    
def download(path,content):
    #browser.switch_to.frame(browser.find_element_by_css_selector('iframe[id="content_frame"]'))
    browser.find_element_by_link_text(content).click();
    i=0
    while 1:
        browser.switch_to.frame(browser.find_element_by_css_selector('iframe[name="content_frame"]'))
        noticelist=browser.find_elements_by_css_selector('tr[class] td a')
        try:
            noticelist[i].click()
        except IndexError:
            break
        i+=1
        time.sleep(2)
        text=browser.find_element_by_css_selector('table[id="table_box"]').text
        name=re.search(pattern_name,text).group(1).strip()
        text=re.search(pattern_text,text).group(1).strip()
        save_txt(text,name,path,renew=0) 
        browser.back()
               
def download_all(content):
    i=0
    current_path=os.getcwd()+os.path.sep+content 
    while 1:
        classlist=browser.find_elements_by_css_selector('table[id="info_1"] tbody tr[class] a')
        try:
            classlist[i].click()
            name=classlist[i].text
        except IndexError:
            break
        target_Path=current_path+os.path.sep+name
        if not os.path.exists(target_Path):  
            os.makedirs(target_Path)
        browser.switch_to_window(browser.window_handles[1])
        i+=1
        download(target_Path,content)
        browser.close()
        browser.switch_to_window(browser.window_handles[0])

def main():
    browser.implicitly_wait(10)
    browser.get('http://learn.tsinghua.edu.cn/')
    login()
    browser.get('http://learn.tsinghua.edu.cn/MultiLanguage/lesson/student/MyCourse.jsp?language=cn')
    #browser.switch_to.frame(browser.find_element_by_css_selector('iframe[id="content_frame"]'))
    #save(browser.page_source,'now')
    #classlist=browser.find_elements_by_css_selector('table[id="info_1"] tbody tr[class] a')
    #browser.switch_to.default_content()
    
    download_all('课程公告' )

main()


