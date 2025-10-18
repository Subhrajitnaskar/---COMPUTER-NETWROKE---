import socket
import struct
import zlib
import random

## ------------------ CONFIGURATION ------------------
HOST = '127.0.0.1'
PORT = 12345
BUFFER_SIZE = 2048
OUTPUT_FILE = 'output_sr.txt'
WINDOW_SIZE = 4 # The 'N' in Selective Repeat

# --- Frame Structure Constants ---
HEADER_FORMAT = '! 6s 6s H B'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
CRC_SIZE = 4
## ----------------------------------------------------

def calculate_crc(data):
    return zlib.crc32(data)

def create_ack(seq_num):
    return b'ACK' + seq_num.to_bytes(1, 'big')

def run_receiver():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((HOST, PORT))
    server_socket.listen(1)
    print(f"✅ Selective Repeat Server is listening on {HOST}:{PORT}")
    print(f"   Window Size (N) = {WINDOW_SIZE}")
    print(f"   Received data will be saved to '{OUTPUT_FILE}'")

    client_socket, client_address = server_socket.accept()
    print(f"\nConnection established from {client_address}")
    
    expected_seq_num = 0
    receive_buffer = {} # Buffer for out-of-order frames
    
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
                
                # --- Selective Repeat Logic ---
                # Check if the frame is within the receiver's window
                if expected_seq_num <= received_seq_num < expected_seq_num + WINDOW_SIZE:
                    # Always send an ACK for a valid frame within the window
                    ack_to_send = create_ack(received_seq_num)
                    client_socket.sendall(ack_to_send)
                    print(f"-> Sent ACK {received_seq_num}")

                    # If it's the expected frame, accept and check buffer for contiguous frames
                    if received_seq_num == expected_seq_num:
                        print(f"   ✅ Frame {expected_seq_num} is in-order. Accepting.")
                        f.write(payload)
                        expected_seq_num += 1
                        
                        # Check buffer for next frames that can now be delivered
                        while expected_seq_num in receive_buffer:
                            print(f"   ✅ Delivering buffered Frame {expected_seq_num}.")
                            buffered_payload = receive_buffer.pop(expected_seq_num)
                            f.write(buffered_payload)
                            expected_seq_num += 1
                        print(f"   Receiver window base slides to {expected_seq_num}")

                    # If it's an out-of-order frame, buffer it
                    else:
                        if received_seq_num not in receive_buffer:
                            print(f"   ⚠️ Frame {received_seq_num} is out-of-order. Buffering.")
                            receive_buffer[received_seq_num] = payload
                
                # If frame is a duplicate of an already delivered frame, just ACK it again
                elif received_seq_num < expected_seq_num:
                    ack_to_send = create_ack(received_seq_num)
                    client_socket.sendall(ack_to_send)
                    print(f"-> Resent ACK for duplicate Frame {received_seq_num}")

    finally:
        client_socket.close()
        server_socket.close()
        print("Connection closed.")

if __name__ == "__main__":
    run_receiver()