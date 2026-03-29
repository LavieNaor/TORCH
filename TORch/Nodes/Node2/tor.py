import socket


class Circuit:
    def __init__(self, circuit_id, session_key, prev_sock, next_sock=None):
        self.circuit_id = circuit_id
        self.session_key = session_key
        self.prev_sock = prev_sock
        self.next_sock = next_sock


class TorNode:
    def __init__(self, host_ip, port):
        self.host_ip = host_ip
        self.port = port

        # circuit_id -> Circuit
        self.circuits = {}

        # init
        self.node_socket = None
