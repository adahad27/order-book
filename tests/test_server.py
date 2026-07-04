import socket
import struct
from typing import List


def build_payload(args: List[str]) -> bytes:
    payload = struct.pack("<I", len(args))
    for arg in args:
        encoded = arg.encode("utf-8")
        payload += struct.pack("<I", len(encoded))
        payload += encoded
    return payload


def recv_all(sock: socket.socket, size: int) -> bytes:
    data = b""
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise ConnectionError("Socket closed while receiving data")
        data += chunk
    return data


def send_order(args: List[str], host: str = "127.0.0.1", port: int = 8080) -> str:
    payload = build_payload(args)
    with socket.create_connection((host, port)) as sock:
        sock.sendall(payload)

        length_bytes = recv_all(sock, 4)
        resp_len = struct.unpack("<I", length_bytes)[0]
        response_bytes = recv_all(sock, resp_len)
        return response_bytes.decode("utf-8")


if __name__ == "__main__":
    first_order = ["add", "AAPL", "user1", "BID", "LIMIT", "175.50", "200"]
    print(send_order(first_order))

    second_order = ["add", "AAPL", "user2", "ASK", "LIMIT", "170.00", "100"]
    third_order = ["add", "AAPL", "user3", "ASK", "LIMIT", "160.00", "100"]

    print(send_order(second_order))
    print(send_order(third_order))