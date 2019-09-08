import socket
import _thread


credentials = {}


def receive_message(conn, client_addr):
    message_received = ""
    while True:
        data_received = conn.recv(16)
        # print("[" + str(client_addr) + "] recebido: " + data_received.decode("utf-8"))
        message_received = message_received + data_received.decode("utf-8")
        if data_received.decode("utf-8").endswith("\r\n"):
            break
    message_received = message_received.split("\r\n")[0]
    return message_received


def authenticate(username, password):
    if username in credentials:
        return credentials[username] == password
    else:
        return False


def check_authentication(conn, client_addr):
    conn.sendall(bytes("user:", 'utf-8'))
    username = receive_message(conn, client_addr)
    # print("[" + str(client_addr) + "] username recebido: " + str(username))

    conn.sendall(bytes("password:", 'utf-8'))
    password = receive_message(conn, client_addr)
    # print("[" + str(client_addr) + "] password recebido: " + str(password))

    return authenticate(username, password)


def manage_command_line(comm, args, conn):
    # Directory Browsing and Listing
    if comm == "cd":
        dirname = args[0]
        print(dirname)

    elif comm == "ls":
        if len(args) != 1:
            dirname = args[0]
            print(dirname)

    elif comm == "pwd":
        pass

    # Directory manipulation
    elif comm == "mkdir":
        dirname = args[0]
        print(dirname)

    elif comm == "rmdir":
        dirname = args[0]
        print(dirname)

    # File Handling
    elif comm == "get":
        filename = args[0]
        print(filename)
    elif comm == "put":
        filename = args[0]
        print(filename)
    elif comm == "delete":
        filename = args[0]
        print(filename)


def control_connection(conn, client_addr):
    print("control connection from" + str(client_addr))

    is_correct_authentication = check_authentication(conn, client_addr)
    conn.sendall(bytes(str(is_correct_authentication), 'utf-8'))
    if is_correct_authentication:
        print("[" + str(client_addr) + "] passou!")

        while True:
            command_line = receive_message(conn, client_addr)
            command = command_line.split(" ")[0]
            command_args = command_line.split(" ")[1:]
            # print("[" + str(client_addr) + "] " + command_line)

            if command == "close" or command == "quit":
                conn.close()
                break
            else:
                manage_command_line(command, command_args, conn)
    else:
        print("[" + str(client_addr) + "] nao passou :(")
        conn.close()


def test_thread(some_text):
    print("example: " + str(some_text))


def create_credentials():
    credentials_file = open("credentials.txt", "r")
    lines = credentials_file.readlines()
    for line in lines:
        username = line.split(" ")[0]
        password = line.split(" ")[1]
        password = password.split("\n")[0]
        credentials[username] = password


if __name__ == "__main__":
    create_credentials()

    # Create a TCP/IP socket and bind the socket to the port
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_address = ('localhost', 2122)
    print("starting up on " + server_address[0] + " port " + str(server_address[1]))
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(1)

    while True:
        # Wait for a connection
        connection, client_address = sock.accept()
        _thread.start_new_thread(control_connection, (connection, client_address))
