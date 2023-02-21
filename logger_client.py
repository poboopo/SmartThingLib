import socket
import struct

group = ("224.1.1.1", 7779)

mainSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
mainSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
mainSocket.bind(group)
mreq = struct.pack("4sl", socket.inet_aton(group[0]), socket.INADDR_ANY)
mainSocket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

try:
    while True:
        print(mainSocket.recv(4096).decode())
except KeyboardInterrupt:
    print("leaving...")
finally:
    mainSocket.close()