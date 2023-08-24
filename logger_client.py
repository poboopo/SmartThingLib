#!/bin/python3.10

import socket
import struct
from datetime import datetime

LOGGER_FILE = "logger.log"
START_COLOR = "\033["
END_COLOR = "\033[0m"

ipNameColor = {}
group = ("224.1.1.1", 7779)
lastColorIndex = 2

mainSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
mainSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
mainSocket.bind(group)
mreq = struct.pack("4sl", socket.inet_aton(group[0]), socket.INADDR_ANY)
mainSocket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

logFile = open(LOGGER_FILE, "a")

def colorByMessageType(messageType):
    if messageType == "ERROR":
        return "31m"
    if messageType == "WARNING":
        return "35m"
    if messageType == "INFO":
        return "34m"
    if messageType == "DEBUG":
        return "32m"
    return "30"

try:
    while True:
        message = mainSocket.recv(4096).decode()
        # that will cause problems
        ipName = message[message.index('{'):message.index('}')+1]
        messageType = message[message.index('}[') + 2:message.index('][')]
        messageCuted = message[message.index('}[') + 1:]

        if (ipName not in ipNameColor.keys()):
            ipNameColor.update({ipName: f"3;9{lastColorIndex}m"})
            lastColorIndex += 1

        formatedMessage = f"[{datetime.now()}]{START_COLOR + ipNameColor[ipName]}{ipName}{END_COLOR}"
        formatedMessage += f"{START_COLOR + colorByMessageType(messageType)}{messageCuted}{END_COLOR}"
        print(formatedMessage)
        logFile.write(formatedMessage + '\n')
        logFile.flush()
except KeyboardInterrupt:
    print("leaving...")
finally:
    mainSocket.close()
    logFile.close()
