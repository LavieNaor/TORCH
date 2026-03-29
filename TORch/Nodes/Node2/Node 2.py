import ssl
import base64
import socket
import json
import threading
import time
import sys
import os
import random
from cryptography.hazmat.primitives.asymmetric import x25519
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
# sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import tor
import aes

LISTEN_PORT = 7100
HOST_IP = "node2"

# create a node class
NODE = tor.TorNode(HOST_IP, LISTEN_PORT)
NODE.node_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# TLS
SERVER_CTX = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
SERVER_CTX.load_cert_chain(certfile="router2.crt", keyfile="router2.key")

CLIENT_CTX = ssl.create_default_context()
CLIENT_CTX.load_verify_locations("ca.crt")
CLIENT_CTX.verify_mode = ssl.CERT_REQUIRED
CLIENT_CTX.check_hostname = False


# Bind and listen to the node socket
def bind_and_listen():
    NODE.node_socket.bind(('0.0.0.0', LISTEN_PORT))
    NODE.node_socket.listen(5)
    print(f"[NODE {LISTEN_PORT}] Listening...", flush=True)


# AES encryption
def encrypt(text, key):
    b = base64.b64encode(text.encode()).decode()
    # print("func enc: " + text)
    ciphertext = aes.encrypt(key, b)
    ciphertext_hex = base64.b64encode(ciphertext).decode()
    return ciphertext_hex


# AES decryption
def decrypt(text, key):
    b = base64.b64decode(text)
    # print("func dec: " + str(b))
    decrypted_msg = aes.decrypt(key, b)
    decrypted_msg_hex = base64.b64decode(decrypted_msg).decode()
    return decrypted_msg_hex


def start_handle_requests():
    bind_and_listen()

    while True:
        # Accept new connection
        print("Waiting for client connection request", flush=True)
        client_soc, client_address = NODE.node_socket.accept()
        print(f"[NODE {LISTEN_PORT}] Connection from {client_address}", flush=True)

        # Start handle new client
        t = threading.Thread(target=handle_handshake, args=(client_soc,))
        t.daemon = True
        t.start()


def recv_message(sock):
    length = sock.recv(10).decode()
    return sock.recv(int(length)).decode()


def send_message(sock, msg_to_send):
    length = len(msg_to_send)
    sock.sendall((str(length).zfill(10) + msg_to_send).encode())


def handle_handshake(client_sock):

    try:
        # choose random id that does not exist
        all_id = range(1, 1000)
        available_id = [n for n in all_id if n not in NODE.circuits]
        circuit_id = random.choice(available_id)

        # First connection with new client / node
        ssl_sock_prev_node = SERVER_CTX.wrap_socket(client_sock, server_side=True)
        data = recv_message(ssl_sock_prev_node)
        print(f"[NODE {LISTEN_PORT}] Data: {data}", flush=True)

        # Get the parameters from the message
        j = json.loads(data)
        client_public = j["client_public"]
        client_public_bytes = base64.b64decode(client_public)

        # Create the private key in Diffie-Hellman
        node_private = x25519.X25519PrivateKey.generate()
        node_public = node_private.public_key()

        node_public_bytes = node_public.public_bytes(
            encoding=serialization.Encoding.Raw,
            format=serialization.PublicFormat.Raw
        )
        msg = base64.b64encode(node_public_bytes).decode()

        shared_secret = node_private.exchange(x25519.X25519PublicKey.from_public_bytes(client_public_bytes))

        # Create AES key from shared_secret
        aes_key = HKDF(
            algorithm=hashes.SHA256(),
            length=16,
            salt=None,
            info=b"tor-session"
        ).derive(shared_secret)

        # Send to the client the AES key
        wrapped = json.dumps({
            "node_public": msg,
            "circuit_id": circuit_id
        })
        send_message(ssl_sock_prev_node, wrapped)

        # Add the new circuit to the dict by the id
        NODE.circuits[circuit_id] = tor.Circuit(circuit_id=circuit_id, session_key=aes_key, prev_sock=ssl_sock_prev_node)
        print("Shared:", shared_secret)
        print("AES key:", aes_key)

        # Handshake done - start handle requests
        print(f"Handshake done! key: {aes_key}", flush=True)
        handle_new_client(circuit_id)

    except Exception as e:
        print(f"[NODE {LISTEN_PORT}] Error: {e}", flush=True)
        client_sock.close()


def handle_new_client(circuit_id):

    # Create a references to the circuit's fields
    circuit = NODE.circuits[circuit_id]
    prev_sock = circuit.prev_sock
    next_sock = None
    s_next = None
    key = base64.b64encode(circuit.session_key).decode()

    try:
        data = recv_message(prev_sock)
        print(f"[NODE {LISTEN_PORT}] Data: {data}", flush=True)

        # Decryption with the circuit's key
        decrypted_msg_hex = decrypt(data, key)

        print(f"[NODE {LISTEN_PORT}] Decrypted: {decrypted_msg_hex}", flush=True)

        # Get the parameters from the message
        j = json.loads(decrypted_msg_hex)
        next_port = int(j["port"])
        next_ip = j["ip"]
        encrypted_msg = j["message"]

        # Create a new TLS socket
        s_next = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s_next.connect((next_ip, next_port))
        circuit.next_sock = CLIENT_CTX.wrap_socket(s_next, server_hostname=next_ip)
        next_sock = circuit.next_sock

        # Send the message to the next socket
        send_message(next_sock, encrypted_msg)

        t1 = threading.Thread(target=next_connection, args=(circuit, key,))
        t1.daemon = True
        t1.start()

        t2 = threading.Thread(target=prev_connection, args=(circuit, key,))
        t2.daemon = True
        t2.start()

    except Exception as e:
        print(f"[NODE {LISTEN_PORT}] Error: {e}", flush=True)
        circuit.prev_sock.close()
        if circuit.next_sock:
            circuit.next_sock.close()


def next_connection(circuit, key):
    try:
        while True:
            # Get reply from the socket
            data = recv_message(circuit.next_sock)
            print(f"[NODE {LISTEN_PORT}] Got reply: {data}", flush=True)

            # Wrap the decrypted message
            wrapped = json.dumps({
                "reply": encrypt(data, key)
            })

            send_message(circuit.prev_sock, wrapped)
    except Exception as e:
        print(f"[NODE {LISTEN_PORT}] Error: {e}", flush=True)
        circuit.prev_sock.close()
        if circuit.next_sock:
            circuit.next_sock.close()


def prev_connection(circuit, key):
    try:
        while True:
            data = recv_message(circuit.prev_sock)
            print(f"[NODE {LISTEN_PORT}] Data: {data}", flush=True)

            # Decryption with the circuit's key
            decrypted_msg_hex = decrypt(data, key)

            print(f"[NODE {LISTEN_PORT}] Decrypted: {decrypted_msg_hex}", flush=True)

            # Get the parameters from the message
            j = json.loads(decrypted_msg_hex)
            encrypted_msg = j["message"]

            send_message(circuit.next_sock, encrypted_msg)
    except Exception as e:
        print(f"[NODE {LISTEN_PORT}] Error: {e}", flush=True)
        circuit.prev_sock.close()
        if circuit.next_sock:
            circuit.next_sock.close()


def main():
    start_handle_requests()


if __name__ == "__main__":
    main()
