import socket
from obswebsocket import obsws, requests
from bs4 import BeautifulSoup 
import pyvirtualcam
import numpy as np
import cv2
import os
functions = ["getChatlog","getExtents","getColor","getText","getGradient_opacity","getBk_color","getFile","getFont","getGradient","getSource","getRead_from_file",
    "getValign","getVertical","getExtents_cy","getExtents_cx","getBk_opacity","getGradient_dir","getChatlog_lines","getGradient_color","getOutline_size","getOutline_color","getOutline","getAlign","getOutline_opacity"]
#Vari
getChatlog = lambda txt:txt.getChatlog()
getExtents = lambda txt:txt.getExtents()
getColor = lambda txt:txt.getColor()
getText = lambda txt:txt.getText()
getGradient_opacity = lambda txt:txt.getGradient_opacity()
getBk_color = lambda txt:txt.getBk_color()
getFile = lambda txt:txt.getFile()
getFont = lambda txt:txt.getFont()
getGradient = lambda txt:txt.getGradient()
getSource = lambda txt:txt.getSource()
getRead_from_file = lambda txt:txt.getRead_from_file()
getValign = lambda txt:txt.getValign()
getVertical = lambda txt:txt.getVertical()
getExtents_cy = lambda txt:txt.getExtents_cy()
getExtents_cx = lambda txt:txt.getExtents_cx()
getBk_opacity = lambda txt:txt.getBk_opacity()
getGradient_dir = lambda txt:txt.getGradient_dir()
getChatlog_lines = lambda txt:txt.getChatlog_lines()
getGradient_color = lambda txt:txt.getGradient_color()
getOutline_size = lambda txt:txt.getOutline_size()
getOutline_color = lambda txt:txt.getOutline_color()
getOutline = lambda txt:txt.getOutline()
getAlign = lambda txt:txt.getAlign()
getOutline_opacity = lambda txt:txt.getOutline_opacity()


host = "localhost"
port = 4444
password = "password"

TEXT_SCORE = "pras_score"
MEDIA_NAME = "pras_goal"

WIDTH = 256
HEIGHT = 256

TextProps = {TEXT_SCORE:{}}
imageTables = {}
npImageTables = {}
lastImg = ""


ws = obsws(host, port, password)
soup = BeautifulSoup(open("./templates/index.html"),"html.parser")
imgSoup = soup.find("img")
#以下ループ

def recvData(data):
    global lastImg
    if data == "scored":
        ws.call(requests.RestartMedia(MEDIA_NAME))
        #changeImage()
        #changeText()
    elif data == "init":
        print("connected!")
    else:# score and usename
        try:
            data = data.split(":")
            name = data[0]
            score = data[1]
            file = imageTables[name]
            if lastImg != file:
                print(file)
                changeImage(file)
            changeText(TEXT_SCORE,score)
            lastImg = file
        except Exception as e:
            print(e)
def changeImage(file):
    path = "images/"+file
    imgSoup["src"] = path
    with open("./templates/index.html",mode="w") as f:
        f.write(str(soup))
    #ws.call(requests.RefreshBrowserSource("pras_icons"))
    ws.call(requests.SetSceneItemRender("pras_icons",render=False))
    ws.call(requests.SetSceneItemRender("pras_icons",render=True))
def initText(source):
    txt = ws.call(requests.GetTextGDIPlusProperties(source))
    for func in functions:
        try:
            TextProps[source][func.replace('get','').lower()] = globals()[func](txt)
        except:
            pass

def changeText(source,text):
    props = TextProps[source]
    props["text"] = text
    res = ws.call(requests.SetTextGDIPlusProperties(**props))
    #res = ws.call(requests.SetTextGDIPlusProperties(source,align,bk_color,bk_opacity,chatlog,chatlog_lines,color,extents,extents_cx,extents_cy,file,read_from_file,font,gradient,gradient_color,gradient_dir,gradient_opacity,outline,outline_color,outline_size,outline_opacity,text,valign,vertical,render=True))
    return res

def readTable():
    global imageTables
    path = "table.txt"
    HEIGHT = 0
    WIDTH = 0
    with open(path,mode="r",encoding="utf-8") as f:
        lines = f.readlines()
        for line in lines:
            if line[0] != "#":#not comment
                #line = re.sub(r"\s+","",line)#空白の削除
                line = line.replace("\n","")#改行の削除
                name,file = line.split(":")
                if name in ["HEIGHT","WIDTH"]:
                    if name is "HEIGHT":
                        HEIGHT = int(line)
                    if name is "WIDTH":
                        WIDTH = int(line)
                    if (HEIGHT != 0) and (WIDTH != 0):
                        setImageSize(WIDTH,HEIGHT)
                    continue
                imageTables[name] = file

def loadIcons():
    for name,path in imageTables.items():
        img = cv2.imread(os.getcwd()+r"/templates/images/"+path)
        #npImageTables[name] = cv2.cvtColor(img,cv2.COLOR_BGR2RGB)
        npImageTables[name] = cv2.cvtColor(cv2.resize(img,(WIDTH,HEIGHT)),cv2.COLOR_BGR2RGB)

def setImageSize(width,height):
    imgSoup["width"] = width
    imgSoup["height"] = height
    with open("./templates/index.html",mode="w") as f:
        f.write(str(soup))
def main():
    readTable()
    setImageSize(WIDTH,HEIGHT)
    #loadIcons()
    print(imageTables)
    ws.connect()
    #init
    initText(TEXT_SCORE)
    print("接続完了")
    with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as s:
        s.bind(("127.0.0.1",12345))
        while 1:
            message, _ = s.recvfrom(256)
            message = message.decode(encoding='utf-8')
            print(message)
            recvData(message)
    ws.disconnect()

main()
