from badge_socket import socket
from sys import print_exception

sock = socket()
sock.bind("0.0.0.0", 9001)
sock.listen()
while True:
    lines = list()
    try:
        conn, addr, port = sock.accept()
        print(f"New connection from {addr}:{port}")
        while line := conn.readline():
            if line.decode().startswith("EOF"):
                break
            lines.append(line.decode())
    except BaseException as e:
        print("Connection closed:", end="")
        print_exception(e)
    if len(lines) > 0:
        print(f"Got {len(lines)} lines, first line {repr(lines[0])}")
        try:
            print(f"Calling exec()")
            exec("".join(lines))
            print(f"exec() succeeded")
        except BaseException as e:
            print("exec() failed: ", end="")
            print_exception(e)
    conn.close()
