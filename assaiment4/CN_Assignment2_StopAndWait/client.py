import socket
import struct
import zlib
import random
import time

## ------------------ CONFIGURATION ------------------
SERVER_HOST = '127.0.0.1'
SERVER_PORT = 12345
BUFFER_SIZE = 2048
TIMEOUT = 2
INPUT_FILE = 'input.txt'

# --- Frame Structure Constants (as per PDF) ---
SRC_ADDR = b'\x12\x34\x56\x78\x9A\xBC'
DEST_ADDR = b'\xDE\xF0\x12\x34\x56\x78'
HEADER_FORMAT = '! 6s 6s H B'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
CRC_SIZE = 4
PAYLOAD_SIZE = 50

# --- Simulation Parameters ---
PACKET_LOSS_PROB = 0.2
## ----------------------------------------------------

def calculate_crc(data):
    return zlib.crc32(data)

def create_frame(seq_num, payload):
    frame_length = HEADER_SIZE + len(payload) + CRC_SIZE
    
    # The sequence number is packed as a single unsigned byte ('B').
    # Using modulo (%) ensures it wraps around from 255 back to 0, preventing errors.
    header = struct.pack(HEADER_FORMAT, SRC_ADDR, DEST_ADDR, frame_length, seq_num % 256)
    
    crc = calculate_crc(header + payload)
    return header + payload + struct.pack('!I', crc)

def run_sender():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print(f"Connected to server at {SERVER_HOST}:{SERVER_PORT}")
    except ConnectionRefusedError:
        print("Connection refused. Is the server running?")
        return

    seq_num = 0
    
    try:
        with open(INPUT_FILE, 'rb') as f:
            while True:
                payload = f.read(PAYLOAD_SIZE)
                if not payload:
                    break

                frame_to_send = create_frame(seq_num, payload)

                ack_received = False
                while not ack_received:
                    if random.random() < PACKET_LOSS_PROB:
                        print(f"-> [SIM] Frame {seq_num} is LOST.")
                        time.sleep(TIMEOUT)
                    else:
                        client_socket.sendall(frame_to_send)
                        print(f"-> Sent Frame {seq_num}")
                    
                    client_socket.settimeout(TIMEOUT)
                    
                    try:
                        ack_data = client_socket.recv(BUFFER_SIZE)
                        if ack_data and ack_data.startswith(b'ACK'):
                            ack_seq_num = int.from_bytes(ack_data[3:], 'big')
                            if ack_seq_num == seq_num:
                                print(f"<- Received ACK {ack_seq_num}. OK.")
                                ack_received = True
                            else:
                                print(f"<- Received wrong ACK {ack_seq_num}. Expecting {seq_num}. Ignoring.")
                        else:
                             print("<- Received invalid ACK. Ignoring.")
                    except socket.timeout:
                        print(f"!! TIMEOUT! No ACK for frame {seq_num}. Retransmitting...")
    
                # Increment the sequence number for the next frame
                seq_num += 1

        print("\nEnd of file reached. Sending termination signal.")
        client_socket.sendall(b'END')

    except FileNotFoundError:
        print(f"❌ Error: {INPUT_FILE} not found. Please create it.")
    finally:
        client_socket.close()
        print("Connection closed.")

if __name__ == "__main__":
    try:
        with open(INPUT_FILE, "a"): pass
    except IOError:
        print(f"Could not create {INPUT_FILE}")
    run_sender()