import socket
import struct
from typing import List
import pandas as pd
from concurrent.futures import ThreadPoolExecutor
import time

DIR = "data"
PATH = f"{DIR}/synthetic_data.csv"
WORKER_COUNT = 4
ITERATIONS = 100_000

df = pd.read_csv(PATH)
# print(df.shape)

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

def read_order(idx : int):
    row = df.iloc[idx]
    order = row.tolist()

    match order[0]:
        case 'add':
            order = order[:-1]
            order[-1] = str(int(order[-1]))
        case 'cancel':
            order = order[:-4]
        case 'modify':
            order[-1] = str(int(order[-1]))
            order[-2] = str(order[-2])
    send_order(order)

if __name__ == "__main__":

    start_time = time.perf_counter()
    with ThreadPoolExecutor(max_workers=WORKER_COUNT) as executor:
        for i in range(ITERATIONS):
            executor.submit(read_order, i)
    end_time = time.perf_counter()

    duration = round(end_time - start_time, 4)

    print(f"All orders processed in {duration}")
    print(f"The server had a processing speed of {ITERATIONS / duration} and each order took {duration / ITERATIONS * 1e6} us on average")
