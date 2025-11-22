import socket
import argparse
import os
import sys

def recvall(sock, n):
    data = b''
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data += packet
    return data

def recv_line(sock):
    line = b''
    while not line.endswith(b'\n'):
        chunk = sock.recv(1)
        if not chunk:
            return None
        line += chunk
    return line.decode().strip()

def handle_ok_payload(sock, header):
    # header like "OK <size>"
    parts = header.split()
    if len(parts) >= 2:
        try:
            size = int(parts[1])
        except:
            print(header)
            return
        data = recvall(sock, size)
        if data is None:
            print("Failed to receive payload")
            return
        print(data.decode())
    else:
        # no size provided, just print header
        print(header)

def cmd_list(sock):
    sock.sendall(b"LS\n")
    header = recv_line(sock)
    if header is None:
        print("Connection lost")
        return
    if header.startswith("OK "):
        handle_ok_payload(sock, header)
    else:
        print(header)

def cmd_get(sock, filename):
    sock.sendall(f"GET {filename}\n".encode())
    header = recv_line(sock)
    if header is None:
        print("Connection lost")
        return
    if header.startswith("OK "):
        size = int(header.split()[1])
        outname = os.path.basename(filename)
        print(f"Receiving {outname} ({size} bytes)")
        received = recvall(sock, size)
        if received is None:
            print("Transfer failed")
            return
        with open(outname, 'wb') as f:
            f.write(received)
        print("Saved as", outname)
    else:
        print(header)

def cmd_put(sock, filename):
    if not os.path.exists(filename) or not os.path.isfile(filename):
        print("Local file not found")
        return
    size = os.path.getsize(filename)
    sock.sendall(f"PUT {os.path.basename(filename)}\n".encode())
    header = recv_line(sock)
    if header is None:
        print("Connection lost")
        return
    if not header.startswith("OK"):
        print(header)
        return
    # Send SIZE header and file
    sock.sendall(f"SIZE {size}\n".encode())
    with open(filename, 'rb') as f:
        while True:
            chunk = f.read(4096)
            if not chunk:
                break
            sock.sendall(chunk)
    final = recv_line(sock)
    if final:
        print(final)
    else:
        print("No final response")

def cmd_pwd(sock):
    sock.sendall(b"PWD\n")
    header = recv_line(sock)
    if header is None:
        print("Connection lost")
        return
    if header.startswith("OK "):
        handle_ok_payload(sock, header)
    else:
        print(header)

def cmd_cwd(sock, arg):
    if not arg:
        print("Usage: CWD <directory>")
        return
    sock.sendall(f"CWD {arg}\n".encode())
    header = recv_line(sock)
    if header is None:
        print("Connection lost")
        return
    if header.startswith("OK "):
        handle_ok_payload(sock, header)
    else:
        print(header)

def interactive(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        print(f"Connected to {host}:{port}")
        try:
            while True:
                line = input("ftp> ").strip()
                if not line:
                    continue
                parts = line.split(maxsplit=1)
                verb = parts[0].upper()
                arg = parts[1] if len(parts) > 1 else ''
                if verb == 'LS':
                    cmd_list(s)
                elif verb == 'GET':
                    if not arg:
                        print("Usage: GET <filename>")
                        continue
                    cmd_get(s, arg)
                elif verb == 'PUT':
                    if not arg:
                        print("Usage: PUT <filename>")
                        continue
                    cmd_put(s, arg)
                elif verb == 'PWD':
                    cmd_pwd(s)
                elif verb == 'CWD':
                    cmd_cwd(s, arg)
                elif verb == 'QUIT':
                    s.sendall(b"QUIT\n")
                    header = recv_line(s)
                    if header:
                        print(header)
                    print("Closing connection.")
                    return
                else:
                    print("Unknown command. Available: LS, GET <file>, PUT <file>, PWD, CWD <dir>, QUIT")
        except KeyboardInterrupt:
            print("\nInterrupted. Sending QUIT.")
            try:
                s.sendall(b"QUIT\n")
            except:
                pass

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Simple FTP-like client with PWD/CWD")
    parser.add_argument('--host', '-H', default='127.0.0.1', help='Server host (default localhost)')
    parser.add_argument('--port', '-p', type=int, default=2121, help='Server port (default 2121)')
    args = parser.parse_args()
    interactive(args.host, args.port)
