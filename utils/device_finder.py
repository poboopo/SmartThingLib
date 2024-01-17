#!/bin/python3.10

import socket
import struct
import threading
from threading import Thread
from time import sleep

GROUP = ("224.1.1.1", 7778)
SEARCH_TIME = 2
foundIps = {}
searching = True

def search():
    mainSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    mainSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    mainSocket.bind(GROUP)
    mreq = struct.pack("4sl", socket.inet_aton(GROUP[0]), socket.INADDR_ANY)
    mainSocket.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    try:
        lastColorIndex = 1
        while searching:
            ip = mainSocket.recv(4096).decode()
            if (ip and ip not in foundIps.keys()):
                foundIps.update({ip: f"\033[9{lastColorIndex}m"})
                lastColorIndex += 1
    finally:
        mainSocket.close()

if __name__ == "__main__":
    searchThread = Thread(target=search, daemon=True)
    searchThread.start()

    print("Searching....")
    countdown = SEARCH_TIME
    try:
        while countdown > 0:
            print(f"Total found: {len(foundIps)}")
            sleep(1)
            countdown -= 1
        print("Search finished!")
    except KeyboardInterrupt as ex:
        print("Trying to stop gracefully")
    finally:
        searching = False
        try:
            searchThread.join()
        except KeyboardInterrupt as ex2:
            print("Force stop :(")

    print("Found devices ips:")
    for i, ip in enumerate(foundIps.keys()):
        print(f"[{i + 1}] :: {foundIps[ip]}{ip}\033[0m")

        