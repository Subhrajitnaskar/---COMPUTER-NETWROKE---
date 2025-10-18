import socket
import struct
import zlib
import random

## ------------------ CONFIGURATION ------------------
HOST = '127.0.0.1'
PORT = 12345
BUFFER_SIZE = 2048
OUTPUT_FILE = 'output.txt'

# --- Frame Structure Constants ---
HEADER_FORMAT = '! 6s 6s H B'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
CRC_SIZE = 4
## ----------------------------------------------------

def calculate_crc(data):
    return zlib.crc32(data)

def create_ack(seq_num):
    # ACK frame format: b'ACK' + sequence number (1 byte)
    return b'ACK' + seq_num.to_bytes(1, 'big')

def run_receiver():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)
    print(f"✅ Go-Back-N Server is listening on {HOST}:{PORT}")
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

                # --- De-framing and Integrity Check ---
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
                
                # --- Go-Back-N Logic: Accept only in-order frames ---
                if received_seq_num == expected_seq_num:
                    print(f"   ✅ Frame {received_seq_num} is expected. Accepting data.")
                    f.write(payload)
                    
                    ack_to_send = create_ack(expected_seq_num)
                    client_socket.sendall(ack_to_send)
                    print(f"-> Sent ACK {expected_seq_num}")

                    # Increment to the next expected frame
                    expected_seq_num = (expected_seq_num + 1) % 256
                else:
                    # If frame is out of order, discard it
                    print(f"   ⚠️ Frame {received_seq_num} is out-of-order (expected {expected_seq_num}). Discarding.")
                    # Resend ACK for the last correctly received frame to help the sender
                    if expected_seq_num > 0:
                        last_ack = create_ack(expected_seq_num - 1)
                        client_socket.sendall(last_ack)
                        print(f"-> Resent ACK for last successful frame: {expected_seq_num - 1}")
    
    finally:
        client_socket.close()
        server_socket.close()
        print("Connection closed.")

if __name__ == "__main__":
    run_receiver()