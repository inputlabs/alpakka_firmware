import socket
import time
import random

def send_udp_message(data, ip, port):
    try:
        # Create a UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        # Send the message
        sock.sendto(data, (ip, port))
        # print(f"Sent message: {data} to {ip}:{port}")
    except Exception as e:
        print(f"Error sending message: {e}")
    finally:
        sock.close()

# Example usage:
if __name__ == "__main__":
    target_ip = "255.255.255.255"
    target_port = 8010


    for i in range(100000):
        f = 100
        j = i % 100
        speed = 10
        if (j < (f/2)): x = 256 - speed
        else: x = speed
        data = bytes([2, 0, x, 0] + ([0]*60) )
        send_udp_message(data, target_ip, target_port)
        jitter = random.uniform(0.0, 0.0050)
        print(jitter)
        time.sleep(jitter)
