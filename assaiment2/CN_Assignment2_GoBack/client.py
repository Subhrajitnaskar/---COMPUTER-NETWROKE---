import socket
import struct
import zlib
import random
import time

## ------------------ CONFIGURATION ------------------
SERVER_HOST = '127.0.0.1'
SERVER_PORT = 12345
BUFFER_SIZE = 2048
TIMEOUT = 3 # seconds
INPUT_FILE = 'input.txt'
WINDOW_SIZE = 4 # The 'N' in Go-Back-N

# --- Frame Structure Constants ---
SRC_ADDR = b'\x12\x34\x56\x78\x9A\xBC'
DEST_ADDR = b'\xDE\xF0\x12\x34\x56\x78'
HEADER_FORMAT = '! 6s 6s H B'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
CRC_SIZE = 4
PAYLOAD_SIZE = 100

# --- Simulation Parameters ---
PACKET_LOSS_PROB = 0.15
## ----------------------------------------------------

def calculate_crc(data):
    return zlib.crc32(data)

def create_frame(seq_num, payload):
    frame_length = HEADER_SIZE + len(payload) + CRC_SIZE
    header = struct.pack(HEADER_FORMAT, SRC_ADDR, DEST_ADDR, frame_length, seq_num % 256)
    crc = calculate_crc(header + payload)
    return header + payload + struct.pack('!I', crc)

def run_sender(sock):
    base = 0
    next_seq_num = 0
    unacknowledged_frames = {}
    timer_start_time = None
    
    ack_buffer = b''
    ack_frame_size = 4 # b'ACK' + 1 byte for seq_num

    f = open(INPUT_FILE, 'rb')
    file_done = False

    while not file_done or unacknowledged_frames:
        # --- 1. Check for a timeout (highest priority) ---
        if timer_start_time and (time.time() - timer_start_time > TIMEOUT):
            print(f"!! TIMEOUT for base frame {base}. Retransmitting window.")
            for seq_num in sorted(unacknowledged_frames.keys()):
                sock.sendall(unacknowledged_frames[seq_num])
                print(f"-> Resent Frame {seq_num}")
            
            # Restart the timer after retransmitting
            timer_start_time = time.time()

        # --- 2. Try to receive an ACK ---
        try:
            sock.settimeout(0.0) # Set to non-blocking
            new_data = sock.recv(BUFFER_SIZE)
            ack_buffer += new_data
        except BlockingIOError:
            # This is the normal case when no data is available
            pass
        except Exception as e:
            print(f"An error occurred while receiving ACKs: {e}")
            break
        
        # --- Process one ACK from the buffer if available ---
        if len(ack_buffer) >= ack_frame_size:
            ack_data = ack_buffer[:ack_frame_size]
            if ack_data.startswith(b'ACK'):
                ack_seq_num = ack_data[3]
                print(f"<- Received Cumulative ACK {ack_seq_num}. OK.")

                if ack_seq_num >= base:
                    # Remove acknowledged frames
                    acked_keys = [k for k in unacknowledged_frames if k <= ack_seq_num]
                    for k in acked_keys:
                        del unacknowledged_frames[k]
                    
                    base = ack_seq_num + 1
                    print(f"   Window base slides to {base}")
                    
                    # Restart timer for the new base frame
                    if unacknowledged_frames:
                        timer_start_time = time.time()
                    else:
                        timer_start_time = None # Stop timer
            
            # Trim the processed ACK from the buffer
            ack_buffer = ack_buffer[ack_frame_size:]

        # --- 3. Send one new frame if the window has space ---
        if next_seq_num < base + WINDOW_SIZE and not file_done:
            payload = f.read(PAYLOAD_SIZE)
            if not payload:
                file_done = True
            else:
                frame_to_send = create_frame(next_seq_num, payload)
                unacknowledged_frames[next_seq_num] = frame_to_send
                
                if random.random() < PACKET_LOSS_PROB:
                    print(f"-> [SIM] Frame {next_seq_num} is LOST.")
                else:
                    sock.sendall(frame_to_send)
                    print(f"-> Sent Frame {next_seq_num}")

                # Start timer if this is the first frame in the window
                if base == next_seq_num:
                    timer_start_time = time.time()
                
                next_seq_num += 1
    
    f.close()
    print("\nEnd of file reached and all ACKs received. Sending termination signal.")
    sock.sendall(b'END')

if __name__ == "__main__":
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print(f"✅ Go-Back-N Client connected to server at {SERVER_HOST}:{SERVER_PORT}")
        print(f"   Window Size (N) = {WINDOW_SIZE}\n")
        run_sender(client_socket)
    except ConnectionRefusedError:
        print("❌ Connection refused. Is the server running?")
    finally:
        client_socket.close()
        print("Connection closed.")