import socket
import threading
from obswebsocket import obsws, requests

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

TextProps = {TEXT_SCORE:{}}
imageTables = {}

ws = obsws(host, port, password)
#
#以下ループ

def recvData(data):
    if data == "scored":
        ws.call(requests.RestartMedia(MEDIA_NAME))
    elif data == "init":
        print("connected!")
    else:# score and usename
        data = data.split(":")
        name = data[0]
        score = data[1]
        file = imageTables[name]
        changeImg(file)
        changeText(TEXT_SCORE,score)

def loop_handler(conn,addr):
    while 1:
        try:
            data = conn.recv(4096)
            #th = threading.Thread(target=recvData,args=(data.decode(),),daemon=True)
            #th.start()
            recvData(data.decode())
            print(f"data:{data.decode()}")
        except Exception as e:
            print(e)
            break

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

def changeImg(file:str):
    import urllib
    url = "localhost:5000/get?name="+file
    res = urllib.request.urlopen(url)
    code = res.getcode()
    res.close()
    return code

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

def main():
    readTable()
    print(imageTables)
    ws.connect()
    #init
    initText(TEXT_SCORE)
    ws.call(requests.CreateSource("imgTst","image_source","DEV",{"file":r"C:\Users\kazum\Desktop\cry.png"},True))
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1",12345))
        s.listen(5)
        print("waiting")
        while 1:
            try:
                conn,addr = s.accept()
            except KeyboardInterrupt:
                break
            print(f"from:{addr}")
            thread = threading.Thread(target=loop_handler,args=(conn,addr),daemon=True)
            thread.start()
    ws.disconnect()

main()
