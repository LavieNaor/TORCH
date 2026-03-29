import aes
import base64
import threading
import socket
import json
import ssl
import random
from cryptography.hazmat.primitives.asymmetric import x25519
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes, serialization

QUEUE_RECEIVE_MESSAGES = []

SERVER_IP = "127.0.0.1"
NODE1_IP = "127.0.0.1"
NODE2_IP = "127.0.0.1"
NODE3_IP = "127.0.0.1"

# Response codes
ERROR_RESPONSE_CODE = 99
SIGNUP_RESPONSE_CODE = 101
LOGIN_RESPONSE_CODE = 102
SIGNOUT_RESPONSE_CODE = 103
CREATE_NEW_GROUP_CHAT_RESPONSE_CODE = 104
CREATE_NEW_SINGLE_CHAT_RESPONSE_CODE = 105
LOAD_CHAT_RESPONSE_CODE = 106
SEND_MESSAGE_RESPONSE_CODE = 107
GET_ALL_USERS_RESPONSE_CODE = 108
ADD_USER_RESPONSE_CODE = 109
REMOVE_USER_RESPONSE_CODE = 110
RECEIVE_MESSAGE_RESPONSE_CODE = 111

# Ports
NODE1_PORT = 7000
NODE2_PORT = 7100
NODE3_PORT = 7200
SERVER_PORT = 8000

# TLS
CLIENT_CTX = ssl.create_default_context()
CLIENT_CTX.load_verify_locations("ca.crt")
CLIENT_CTX.verify_mode = ssl.CERT_REQUIRED
CLIENT_CTX.check_hostname = False


class Node:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.key = None
        self.circuit_id = None


NODE1 = Node(NODE1_IP, NODE1_PORT)
NODE2 = Node(NODE2_IP, NODE2_PORT)
NODE3 = Node(NODE3_IP, NODE3_PORT)


class Client:
    def __init__(self, entry_node, middle_node, exit_node):

        # node class
        self.entry_node = entry_node
        self.middle_node = middle_node
        self.exit_node = exit_node

        # socket
        self.entry_node_socket = None


def encrypt(text, key):
    ciphertext1 = aes.encrypt(key, base64.b64encode(text.encode()).decode())
    ciphertext1hex = base64.b64encode(ciphertext1).decode()
    return ciphertext1hex


def decrypt(text, key):
    decrypted_msg = aes.decrypt(key, base64.b64decode(text))
    decrypted_msg_hex = base64.b64decode(decrypted_msg).decode()
    return decrypted_msg_hex


def build_onion_with_ports(message, client):
    enc3 = build_layer_with_port(message, client.exit_node.key, SERVER_IP, SERVER_PORT)
    enc2 = build_layer_with_port(enc3, client.middle_node.key, client.exit_node.ip, client.exit_node.port)
    enc1 = build_layer_with_port(enc2, client.entry_node.key, client.middle_node.ip, client.middle_node.port)

    return enc1


def build_onion_without_ports(message, client):
    enc3 = build_layer_without_port(message, client.exit_node.key)
    enc2 = build_layer_without_port(enc3, client.middle_node.key)
    enc1 = build_layer_without_port(enc2, client.entry_node.key)

    return enc1


def build_layer_with_port(message, key, ip, port):
    layer = {
        "ip": ip,
        "port": port,
        "message": message
    }
    enc = encrypt(json.dumps(layer), base64.b64encode(key).decode())

    return enc


def build_layer_without_port(message, key):
    layer = {
        "message": message
    }
    enc = encrypt(json.dumps(layer), base64.b64encode(key).decode())

    return enc


def receive_ans(sock, client):
    while True:
        ans = sock.recv(4096).decode()
        reply = peel_reply(ans, client)
        if ord(reply[0]) != RECEIVE_MESSAGE_RESPONSE_CODE:
            QUEUE_RECEIVE_MESSAGES.append(reply)
        else:
            print(reply)


def peel_reply(raw_reply, client):
    # כל צומת עוטף JSON {"reply": <encrypted>}
    current = raw_reply

    key1 = client.entry_node.base64.b64encode(key).decode()
    key2 = client.middle_node.base64.b64encode(key).decode()
    key3 = client.exit_node.base64.b64encode(key).decode()

    for key in [key1, key2, key3]:
        # print(key[-1] + " ----- " + current)
        j = json.loads(current)
        enc = j["reply"]
        dec = decrypt(enc, key)
        current = dec
    return current


def login(client, username, password):

    sock = client.entry_node_socket
    msg = '{"username": "' + username + '", "password": "' + password + '"}'
    length = "{:05d}".format(len(msg))
    msg = '2' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def signup(client, username, password, email):
    sock = client.entry_node_socket
    msg = '{"username": "' + username + '", "password": "' + password + '", "email": "' + email + '"}'
    length = "{:05d}".format(len(msg))
    msg = '1' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def logout(client):
    sock = client.entry_node_socket
    msg = '{}'
    length = "{:05d}".format(len(msg))
    msg = '3' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def new_single_chat(client, username_to_create_chat_with):

    sock = client.entry_node_socket
    msg = '{"username": "' + username_to_create_chat_with + '"}'
    length = "{:05d}".format(len(msg))
    msg = '5' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def new_group_chat(client, group_name, usernames):

    sock = client.entry_node_socket
    # msg = '{"usernames": "' + usernames + '", "groupName": "' + group_name + '"}'
    msg_dict = {"usernames": usernames, "groupName": group_name}
    msg = json.dumps(msg_dict)
    length = "{:05d}".format(len(msg))
    msg = '4' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def load_chat(client, chat_id):
    sock = client.entry_node_socket
    msg = '{"chatID": ' + str(chat_id) + '}'
    length = "{:05d}".format(len(msg))
    msg = '6' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def send_message(client, chat_id, content, is_txt_msg):
    sock = client.entry_node_socket
    msg = '{"chatID": ' + str(chat_id) + ', "content": "' + content + '", "isTxtMsg": ' + str(is_txt_msg) + '}'
    length = "{:05d}".format(len(msg))
    msg = '7' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def get_all_users(client):
    sock = client.entry_node_socket
    msg = '{}'
    length = "{:05d}".format(len(msg))
    msg = '8' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def add_user(client, username_to_add, chat_id):
    sock = client.entry_node_socket
    msg = '{"username_to_add": "' + username_to_add + '", "chatID": "' + chat_id + '"}'
    length = "{:05d}".format(len(msg))
    msg = '9' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def remove_user(client, username_to_remove, chat_id):
    sock = client.entry_node_socket
    msg = '{"username_to_remove": "' + username_to_remove + '", "chatID": "' + chat_id + '"}'
    length = "{:05d}".format(len(msg))
    msg = '10' + length + msg
    onion = build_onion_with_ports(msg, client)
    sock.sendall(onion.encode())

    while len(QUEUE_RECEIVE_MESSAGES) == 0:
        continue

    return QUEUE_RECEIVE_MESSAGES.pop()


def format_to_hex(public_key):
    # format to hex
    public_bytes = public_key.public_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PublicFormat.Raw
    )

    return public_bytes


def get_data_in_handshake(client, num_of_layers):
    private_key = x25519.X25519PrivateKey.generate()
    public_key = private_key.public_key()

    # wrap a json
    public_key_layer = json.dumps({
        "client_public": base64.b64encode(format_to_hex(public_key)).decode()
    })

    layers = create_num_of_layers(public_key_layer, client, num_of_layers)

    # send the public key to the entry node
    client.entry_node_socket.sendall(layers.encode())

    # recv the data
    data = client.entry_node_socket.recv(4096).decode()

    # load the node json
    print("data: " + data)

    return private_key, data


def release_layer_of_onion_answer(data, key):
    j = json.loads(data)
    enc = j["reply"]
    print("key: " + base64.b64encode(key).decode())
    dec = decrypt(enc, base64.b64encode(key).decode())

    return dec


def handshake_with_one_node(client, num_of_layers):
    private_key, data = get_data_in_handshake(client, num_of_layers)

    j = release_num_of_layers(data, client, num_of_layers)

    # get the node public key
    node_public = base64.b64decode(j["node_public"])

    # Create AES key from shared_secret
    shared_secret = private_key.exchange(x25519.X25519PublicKey.from_public_bytes(node_public))
    aes_key = HKDF(
        algorithm=hashes.SHA256(),
        length=16,
        salt=None,
        info=b"tor-session"
    ).derive(shared_secret)

    return aes_key


def create_num_of_layers(first_layer, client, num):
    enc = first_layer
    if num == 0:
        return enc
    if num >= 3:
        enc = build_layer_with_port(enc, client.exit_node.key, SERVER_IP, SERVER_PORT)
    if num >= 2:
        enc = build_layer_with_port(enc, client.middle_node.key, client.exit_node.ip, client.exit_node.port)
    if num >= 1:
        enc = build_layer_with_port(enc, client.entry_node.key, client.middle_node.ip, client.middle_node.port)

    return enc


def release_num_of_layers(data, client, num):

    msg = data

    if num >= 1:
        msg = release_layer_of_onion_answer(msg, client.entry_node.key)
    if num >= 2:
        msg = release_layer_of_onion_answer(msg, client.middle_node.key)
    if num >= 3:
        msg = release_layer_of_onion_answer(msg, client.exit_node.key)

    j = json.loads(msg)

    return j


def handle_handshake(client):

    # --------------------------------------------------------------------------------------------------- #

    # Entry node key

    client.entry_node.key = handshake_with_one_node(client, 0)

    print(f"handshake with entry node finish: {client.entry_node.key}")

    # --------------------------------------------------------------------------------------------------- #

    # Middle node key

    client.middle_node.key = handshake_with_one_node(client, 1)

    print(f"handshake with middle node finish: {client.middle_node.key}")

    # --------------------------------------------------------------------------------------------------- #

    # Exit node key

    client.exit_node.key = handshake_with_one_node(client, 2)

    print(f"handshake with exit node finish: {client.exit_node.key}")

    # --------------------------------------------------------------------------------------------------- #

    print("Handshake done!\n")


def handle_client(CLIENT):
    while True:
        choice = int(input("Choose one option:\n1 - login\n2 - signup\n3 - Quit\nEnter Your Choice: "))

        if choice == 1:
            username = input("Enter Username: ")
            password = input("Enter Password: ")
            replay = login(CLIENT, username, password)
            print(replay)
            if replay[0] != 'c':
                break

        elif choice == 2:
            username = input("Enter Username: ")
            password = input("Enter Password: ")
            email = input("Enter Email: ")
            replay = signup(CLIENT, username, password, email)
            print(replay)
            if replay[0] != 'c':
                break

        elif choice == 3:
            break

        else:
            print("Wrong output!!!")

    while True:
        choice = int(input("Choose one option:\n3 - sign out\n4 - create new group chat\n5 - create new single chat\n6 - load chat\n7 - send message\n8 - get all users\n9 - add user\n10 - remove user\nEnter Your Choice: "))

        if choice == 3:
            logout(CLIENT)
            break

        elif choice == 4:
            group_name = input("Enter group name: ")
            names = []

            while True:
                name = input("Enter username (or 'exit'): ")
                if name == "exit":
                    break
                names.append({"name": name})

            print(new_group_chat(CLIENT, group_name, names))

        elif choice == 5:
            new_username = input("Enter the username to open chat with: ")
            print(new_single_chat(CLIENT, new_username))

        elif choice == 6:
            chat_id = input("Enter chat_id:")
            print(load_chat(CLIENT, chat_id))

        elif choice == 7:
            chat_id = input("Enter chat_id: ")
            content = input("Enter the content of the msg: ")
            print(send_message(CLIENT, chat_id, content, 1))

        elif choice == 8:
            print(get_all_users(CLIENT))

        elif choice == 9:
            username_to_add = input("Enter username to add: ")
            chat_id = input("Enter chat_id: ")
            print(add_user(CLIENT, username_to_add, chat_id))

        elif choice == 10:
            username_to_remove = input("Enter username to remove: ")
            chat_id = input("Enter chat_id: ")
            print(remove_user(CLIENT, username_to_remove, chat_id))

        else:
            print("Wrong output!!!")

    CLIENT.entry_node_socket.close()


def main():

    # Create the nodes
    nodes = [NODE1, NODE2, NODE3]

    # create random orbit
    all_nums = range(0, len(nodes))
    client_list_nodes_index = []

    for i in range(3):
        available_num = [n for n in all_nums if n not in client_list_nodes_index]
        node_index = random.choice(available_num)
        client_list_nodes_index.append(node_index)
        print(node_index)

    entry_node_index = client_list_nodes_index[0]
    middle_node_index = client_list_nodes_index[1]
    exit_node_index = client_list_nodes_index[2]

    CLIENT = Client(nodes[entry_node_index], nodes[middle_node_index], nodes[exit_node_index])

    # it is for the docker to connect with it
    CLIENT.entry_node.ip = '127.0.0.1'

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((CLIENT.entry_node.ip, CLIENT.entry_node.port))
    ssl_sock_next_node = CLIENT_CTX.wrap_socket(sock, server_hostname=SERVER_IP)
    CLIENT.entry_node_socket = ssl_sock_next_node

    # Create nodes handshake
    handle_handshake(CLIENT)

    t = threading.Thread(target=receive_ans, args=(CLIENT.entry_node_socket, CLIENT,))
    t.daemon = True
    t.start()

    t = threading.Thread(target=handle_client, args=(CLIENT,))
    t.daemon = False
    t.start()


if __name__ == '__main__':
    main()
