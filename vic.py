import socket
with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as s:
    s.connect(("127.0.0.1",12345))
    s.sendall(b"2:100")