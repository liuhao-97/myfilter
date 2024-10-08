import socket
import time
import struct

def send_udp_packet(target_ip, target_port, message,):
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        sock.sendto(message, (target_ip, target_port))
        time.sleep(0.0000001)
    except Exception as e:
        print(f"Error sending packet: {e}")
    finally:
        # Close the socket
        sock.close()

if __name__ == "__main__":
    # Replace with the target IP address and port
    target_ip = "10.68.187.36"
    target_port = 9000

    jetson_receive_large_number=3
    jetson_receive_small_number=1
    jetson_receive_large_datasize = 2000
    jetson_receive_small_datasize = 1000
    data_list = ['A','B','C','D']
    message = b''
    for i in range(jetson_receive_large_number):
        
        payload = data_list[i]*jetson_receive_large_datasize
        message = message + payload.encode('utf-8')
    
    send_udp_packet(target_ip, target_port, message)
