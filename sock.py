import socket
from obswebsocket import obsws, requests

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

WIDTH = 500
HEIGHT = 500

TextProps = {TEXT_SCORE:{}}
imageTables = {}
npImageTables = {}
lastImg = ""


ws = obsws(host, port, password)

#以下ループ

def recvData(data,cam):
    global lastImg
    if data == "scored":
        ws.call(requests.RestartMedia(MEDIA_NAME))
    elif data == "init":
        print("connected!")
    else:# score and usename
        data = data.split(":")
        name = data[0]
        score = data[1]
        file = imageTables[name]
        if lastImg != file:
            cam.send(npImageTables[name])
        changeText(TEXT_SCORE,score)
        lastImg = file



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
    import re
    global imageTables
    path = "table.txt"
    with open(path,mode="r") as f:
        lines = f.readlines()
        for line in lines:
            if line[0] != "#":#not comment
                line = re.sub(r"\s+","",line)#空白の削除
                line = line.replace("\n","")#改行の削除
                name,file = line.split(":")
                imageTables[name] = file

def loadIcons():
    for name,path in imageTables.items():
        img = cv2.imread(os.getcwd()+r"/images/"+path)
        #npImageTables[name] = img
        npImageTables[name] = cv2.cvtColor(cv2.resize(img,(WIDTH,HEIGHT)),cv2.COLOR_BGR2RGB)


def main():
    readTable()
    loadIcons()
    print(imageTables)
    ws.connect()
    #init
    initText(TEXT_SCORE)
    print("接続完了")
    with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as s:
        with pyvirtualcam.Camera(width=WIDTH, height=HEIGHT, fps=20) as cam:
            s.bind(("127.0.0.1",12345))
            frame = np.zeros((cam.height, cam.width, 3), np.uint8) # RGBA
            frame[:] = cam.frames_sent % 255 # grayscale animation
            if "TEMP" in imageTables.keys():
                frame[:] = npImageTables["TEMP"]
            cam.send(frame)
            while 1:
                message, _ = s.recvfrom(256)
                message = message.decode(encoding='utf-8')
                print(message)
                recvData(message,cam)
    ws.disconnect()

main()
