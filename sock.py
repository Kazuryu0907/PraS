import socket

with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
    s.bind(("127.0.0.1",12345))
    s.listen(1)
    print("waiting")
    while 1:
        conn,addr = s.accept()
        print(f"from:{addr}")
        data = conn.recv(256)
        print(f"data:{data.decode()}")