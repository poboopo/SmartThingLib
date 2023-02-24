import socket
import struct
from datetime import datetime

ipColor = {}
group = ("224.1.1.1", 7779)
lastColorIndex = 1

mainSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
mainSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
mainSocket.bind(group)
mreq = struct.pack("4sl", socket.inet_aton(group[0]), socket.INADDR_ANY)
mainSocket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

try:
    while True:
        message = mainSocket.recv(4096).decode()
        ip = message[1:message.index('}')]

        if (ip not in ipColor.keys()):
            ipColor.update({ip: f"\033[9{lastColorIndex}m"})
            lastColorIndex += 1

        print(f"{ipColor[ip]}[{datetime.now()}]{message}\033[0m")
except KeyboardInterrupt:
    print("leaving...")
finally:
    mainSocket.close()