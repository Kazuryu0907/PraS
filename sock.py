from http import client
import socket
import threading
from tkinter.tix import TEXT

from obswebsocket import obsws, requests
functions = ["getChatlog","getExtents","getColor","getText","getGradient_opacity","getBk_color","getFile","getFont","getGradient","getSource","getRead_from_file",
    "getValign","getVertical","getExtents_cy","getExtents_cx","getBk_opacity","getGradient_dir","getChatlog_lines","getGradient_color","getOutline_size","getOutline_color","getOutline","getAlign","getOutline_opacity"]
#Vari
chatlog = None
extents = None
color = None
gradient_opacity = None
bk_color = None
file = None
font = None
gradient = None
source = None
read_from_file = None
valign = None
align = None
vertical = None
extents_cy = None
extents_cx = None
bk_opacity = None
gradient_dir = None
chatlog_lines = None
gradient_color = None
outline_size = None
outline_color = None
outline = None
outline_opacity = None


host = "localhost"
port = 4444
password = "password"

TEXT_NAME = "pras_name"
TEXT_SCORE = "pras_score"
MEDIA_NAME = "pras_goal"

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
        score = int(data[1])
        changeText(TEXT_NAME,name)
        changeText(TEXT_SCORE,score)

def loop_handler(conn,addr):
    while 1:
        try:
            data = conn.recv(256)
            recvData(data.decode())
            print(f"data:{data.decode()}")
        except Exception as e:
            print(e)
            break

def initText(source):
    txt = ws.call(requests.GetTextGDIPlusProperties(source))
    for func in functions:
        try:
            exec(f"{func.replace('get','').lower()} = txt.{func}()")
        except:
            pass
    print(source)

def changeText(source,text):
    res = ws.call(requests.SetTextGDIPlusProperties(source,align,bk_color,bk_opacity,chatlog,chatlog_lines,color,extents,extents_cx,extents_cy,file,read_from_file,font,gradient,gradient_color,gradient_dir,gradient_opacity,outline,outline_color,outline_size,outline_opacity,text,valign,vertical,render=True))
    return res

def main():
    ws.connect()
    #init
    initText(TEXT_NAME)
    initText(TEXT_SCORE)
    with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1",12345))
        s.listen(10)
        print("waiting")
        while 1:
            try:
                conn,addr = s.accept()
            except KeyboardInterrupt:
                exit()
                break
            print(f"from:{addr}")
            thread = threading.Thread(target=loop_handler,args=(conn,addr),daemon=True)
            thread.start()
    ws.disconnect()

main()