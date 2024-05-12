#!/bin/python3.10

import socket
import struct
from datetime import datetime
import threading

# connect to udp via param

TCP = True
PORT = 7779
ADRESS = "192.168.2.114"

LOGGER_FILE = "logger.log"
START_COLOR = "\033["
END_COLOR = "\033[0m"

ipColor = {}
lastColorIndex = 2
logFile = open(LOGGER_FILE, "a")

def colorByLevel(logLevel):
    print(logLevel)
    if logLevel == 40:
        return "31m"
    if logLevel == 30:
        return "35m"
    if logLevel == 20:
        return "34m"
    if logLevel == 10:
        return "32m"
    return "30"

def udp():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("224.1.1.1", PORT))
    mreq = struct.pack("4sl", socket.inet_aton(group[0]), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    printHeader()
    try:
        recvMessages("", sock)
    except KeyboardInterrupt:
        print("leaving...")
    finally:
        sock.close()

def tcp():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((ADRESS, PORT))
    sock.listen()
    print("Waiting for connection...")
    printHeader()
    try:
        while(True):
            conn, (ip, port) = sock.accept()
            th = threading.Thread(target=recvMessages, args=[ip, conn], daemon=True)
            th.start()
    except KeyboardInterrupt:
        print("leaving...")
    finally:
        sock.close()

def printHeader():
    print(f"{START_COLOR + '1;36m'}{'':->115}")
    print(f"{'TIMESTAMP': ^26} [{'IP': ^15} :: {'NAME': ^15}] - [{'LEVEL': ^7}] [{'TAG': ^20}] :: LOG MESSAGE")
    print(f"{'':-<115}{END_COLOR}")

def recvMessages(ip, conn):
    global ipColor
    global lastColorIndex
    
    try:
        while True:
            try:
                data = conn.recv(4096)
                if (not data):
                    print("Connection closed")
                    break
                message = data.decode()
                splitted = message.split("_&_")
                if (len(splitted) == 1):
                    print(message)
                    continue

                name = splitted[0]
                logLevel = splitted[1]
                tag = splitted[2]
                messageCuted = splitted[3].strip()

                if (ip not in ipColor.keys()):
                    ipColor.update({ip: f"3{lastColorIndex}m"})
                    lastColorIndex += 1

                formatedMessage = f"{START_COLOR + ipColor[ip]}{datetime.now()} [{ip: ^15} :: {name: ^15}]{END_COLOR} - "
                formatedMessage += f"{START_COLOR + colorByLevel(logLevel)}[{str(logLevel): ^2}] [{tag: ^20}] :: {messageCuted}{END_COLOR}"
                print(formatedMessage)
                logFile.write(formatedMessage + '\n')
                logFile.flush()
            except Exception as e:
                print(f"Failed to process message: {e}")
    finally:
        conn.close()

if __name__ == "__main__":
    tcp()
    logFile.close()
