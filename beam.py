import socket
import time
from soco import SoCo

my_zone = SoCo('192.168.178.27')

while True:
    
    info = my_zone.get_current_track_info()
    print(info)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto("{};{}".format(info['title'], info['artist']).encode(), ("192.168.178.73", 2390))
    time.sleep(4)