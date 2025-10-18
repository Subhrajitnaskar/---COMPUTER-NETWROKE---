import socket
import struct
import zlib
import random

## ------------------ CONFIGURATION ------------------
HOST = '127.0.0.1'
PORT = 12345
BUFFER_SIZE = 2048
OUTPUT_FILE = 'output.txt'

# --- Frame Structure Constants (as per PDF) ---
HEADER_FORMAT = '! 6s 6s H B'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
CRC_SIZE = 4

# --- Simulation Parameters ---
ACK_LOSS_PROB = 0.2
## ----------------------------------------------------

def calculate_crc(data):
    return zlib.crc32(data)

def create_ack(seq_num):
    return b'ACK' + seq_num.to_bytes(1, 'big')

def run_receiver():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)
    print(f"Server is listening on {HOST}:{PORT}")
    print(f"   Received data will be saved to '{OUTPUT_FILE}'")

    client_socket, client_address = server_socket.accept()
    print(f"\nConnection established from {client_address}")
    
    expected_seq_num = 0
    
    try:
        with open(OUTPUT_FILE, 'wb') as f:
            while True:
                frame = client_socket.recv(BUFFER_SIZE)
                if not frame or frame == b'END':
                    print("\nTermination signal received. Shutting down.")
                    break

                if len(frame) < HEADER_SIZE + CRC_SIZE:
                    print(f"❌ Received a malformed frame. Discarding.")
                    continue
                
                header_data = frame[:HEADER_SIZE]
                payload = frame[HEADER_SIZE:-CRC_SIZE]
                received_crc = struct.unpack('!I', frame[-CRC_SIZE:])[0]

                calculated_crc = calculate_crc(header_data + payload)
                if received_crc != calculated_crc:
                    print("❌ CRC mismatch! Frame is corrupt. Discarding.")
                    continue

                _, _, _, received_seq_num = struct.unpack(HEADER_FORMAT, header_data)
                
                print(f"<- Received Frame {received_seq_num}")
                
                if received_seq_num == expected_seq_num:
                    print(f"   Frame {received_seq_num} is expected. Accepting data.")
                    f.write(payload)
                    
                    ack_to_send = create_ack(expected_seq_num)
                    
                    k = random.random()
                    print(f"Random value k = {k:.4f}") # Printing k to see the value
                    if k < ACK_LOSS_PROB:
                        print(f"   [SIM] ACK {expected_seq_num} is LOST (since {k:.4f} < {ACK_LOSS_PROB}).")
                    else:
                        client_socket.sendall(ack_to_send)
                        print(f"-> Sent ACK {expected_seq_num}")

                    # Increment the sequence number
                    expected_seq_num += 1
                else:
                    print(f"   Frame {received_seq_num} is not expected (expected {expected_seq_num}).")
                    # Handle re-sending the ACK for the last successful frame
                    if received_seq_num < expected_seq_num:
                        last_ack_num = received_seq_num
                        ack_to_send = create_ack(last_ack_num)
                        print(f"-> Detected duplicate. Resending ACK {last_ack_num}.")
                        client_socket.sendall(ack_to_send)
    
    finally:
        client_socket.close()
        server_socket.close()
        print("Connection closed.")

if __name__ == "__main__":
    run_receiver()