import socket
import threading
import os
import argparse
import pathlib

HOST = '0.0.0.0'

def recvall(conn, n):
    data = b''
    while len(data) < n:
        packet = conn.recv(n - len(data))
        if not packet:
            return None
        data += packet
    return data

def safe_within_base(base_dir, path):
    # Ensure path is inside base_dir
    base_dir = os.path.abspath(base_dir)
    path = os.path.abspath(path)
    return os.path.commonpath([base_dir]) == os.path.commonpath([base_dir, path])

def handle_client(conn, addr, base_dir):
    print(f"[+] Connection from {addr}")
    current_dir = os.path.abspath(base_dir)  # per-connection cwd
    try:
        with conn:
            while True:
                # Read a command line (terminated by \n)
                cmd_line = b''
                while not cmd_line.endswith(b'\n'):
                    chunk = conn.recv(1)
                    if not chunk:
                        print(f"[-] {addr} disconnected")
                        return
                    cmd_line += chunk
                cmd = cmd_line.decode().strip()
                if not cmd:
                    continue

                parts = cmd.split(maxsplit=1)
                verb = parts[0].upper()
                arg = parts[1] if len(parts) > 1 else ''

                if verb == 'LS':
                    try:
                        entries = os.listdir(current_dir)
                        payload = '\n'.join(entries)
                        payload_bytes = payload.encode()
                        header = f"OK {len(payload_bytes)}\n".encode()
                        conn.sendall(header)
                        conn.sendall(payload_bytes)
                    except Exception as e:
                        conn.sendall(f"ERR {str(e)}\n".encode())

                elif verb == 'GET':
                    if not arg:
                        conn.sendall(b"ERR missing filename\n")
                        continue
                    safe_name = os.path.basename(arg)
                    path = os.path.join(current_dir, safe_name)
                    if not os.path.exists(path) or not os.path.isfile(path):
                        conn.sendall(b"ERR file not found\n")
                        continue
                    try:
                        size = os.path.getsize(path)
                        conn.sendall(f"OK {size}\n".encode())
                        with open(path, 'rb') as f:
                            while True:
                                chunk = f.read(4096)
                                if not chunk:
                                    break
                                conn.sendall(chunk)
                    except Exception as e:
                        conn.sendall(f"ERR {str(e)}\n".encode())

                elif verb == 'PUT':
                    if not arg:
                        conn.sendall(b"ERR missing filename\n")
                        continue
                    safe_name = os.path.basename(arg)
                    path = os.path.join(current_dir, safe_name)
                    conn.sendall(b"OK ready for size\n")
                    # read header line for SIZE
                    header_line = b''
                    while not header_line.endswith(b'\n'):
                        chunk = conn.recv(1)
                        if not chunk:
                            conn.sendall(b"ERR connection lost\n")
                            return
                        header_line += chunk
                    header_line = header_line.decode().strip()
                    if not header_line.startswith('SIZE '):
                        conn.sendall(b"ERR expected SIZE header\n")
                        continue
                    try:
                        size = int(header_line.split()[1])
                    except:
                        conn.sendall(b"ERR bad size\n")
                        continue
                    data = recvall(conn, size)
                    if data is None:
                        conn.sendall(b"ERR transfer failed\n")
                        return
                    with open(path, 'wb') as f:
                        f.write(data)
                    conn.sendall(b"OK uploaded\n")

                elif verb == 'PWD':
                    # send current directory relative to base_dir
                    try:
                        base_abs = os.path.abspath(base_dir)
                        cur_abs = os.path.abspath(current_dir)
                        # represent as path relative to base_dir, leading '/'
                        rel = os.path.relpath(cur_abs, base_abs)
                        if rel == '.':
                            rel_path = '/'
                        else:
                            rel_path = '/' + rel.replace('\\', '/')
                        payload_bytes = rel_path.encode()
                        conn.sendall(f"OK {len(payload_bytes)}\n".encode())
                        conn.sendall(payload_bytes)
                    except Exception as e:
                        conn.sendall(f"ERR {str(e)}\n".encode())

                elif verb == 'CWD':
                    # change working directory for this connection (must stay inside base_dir)
                    if not arg:
                        conn.sendall(b"ERR missing directory\n")
                        continue
                    try:
                        # If arg starts with '/', treat it as relative to base_dir root
                        if arg.startswith('/') or arg.startswith('\\'):
                            candidate = os.path.join(base_dir, arg.lstrip('/\\'))
                        else:
                            candidate = os.path.join(current_dir, arg)
                        candidate = os.path.abspath(candidate)
                        # Verify candidate is a directory and inside base_dir
                        if not os.path.isdir(candidate):
                            conn.sendall(b"ERR not a directory\n")
                            continue
                        if not safe_within_base(base_dir, candidate):
                            conn.sendall(b"ERR access denied\n")
                            continue
                        current_dir = candidate
                        # send new PWD-like response
                        base_abs = os.path.abspath(base_dir)
                        cur_abs = os.path.abspath(current_dir)
                        rel = os.path.relpath(cur_abs, base_abs)
                        rel_path = '/' if rel == '.' else '/' + rel.replace('\\', '/')
                        payload_bytes = rel_path.encode()
                        conn.sendall(f"OK {len(payload_bytes)}\n".encode())
                        conn.sendall(payload_bytes)
                    except Exception as e:
                        conn.sendall(f"ERR {str(e)}\n".encode())

                elif verb == 'QUIT':
                    conn.sendall(b"OK bye\n")
                    print(f"[+] {addr} closed connection")
                    return

                else:
                    conn.sendall(b"ERR unknown command\n")
    except Exception as e:
        print(f"[!] Error with {addr}: {e}")

def start_server(port, base_dir):
    base_dir = os.path.abspath(base_dir)
    pathlib.Path(base_dir).mkdir(parents=True, exist_ok=True)
    print(f"Serving directory: {base_dir} on port {port}")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, port))
        s.listen(5)
        print(f"[+] Listening on {HOST}:{port}")
        while True:
            conn, addr = s.accept()
            t = threading.Thread(target=handle_client, args=(conn, addr, base_dir), daemon=True)
            t.start()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Simple FTP-like server with PWD/CWD")
    parser.add_argument('--port', '-p', type=int, default=2121, help='Port to listen on (default 2121)')
    parser.add_argument('--dir', '-d', default='server_files', help='Directory to serve (default: server_files)')
    args = parser.parse_args()
    start_server(args.port, args.dir)
