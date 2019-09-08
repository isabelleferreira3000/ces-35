import socket
import _thread
import os
from Session import Session
import shutil


credentials = {}
sessions = {}


def receive_message(conn):
    message_received = ""
    while True:
        data_received = conn.recv(16)
        message_received = message_received + data_received.decode("utf-8")
        if message_received.endswith("\r\n"):
            break
    message_received = message_received.split("\r\n")[0]
    return message_received


def authenticate(username, password):
    if username in credentials:
        return credentials[username] == password
    else:
        return False


def check_authentication(conn):
    conn.sendall(bytes("user:\r\n", 'utf-8'))
    username = receive_message(conn)

    conn.sendall(bytes("password:\r\n", 'utf-8'))
    password = receive_message(conn)

    return authenticate(username, password), username


def check_if_file_already_exists(conn, filename):
    files = os.listdir(sessions[conn].current_directory)
    if filename in files:
        conn.sendall(bytes("file already exists\r\n", 'utf-8'))
        return True
    else:
        conn.sendall(bytes("file does not exists yet\r\n", 'utf-8'))
        return False


def manage_command_line(comm, args, conn):
    curr_session = sessions[conn]

    # Directory Browsing and Listing
    if comm == "cd":
        dirname = args[0]
        curr_path = curr_session.current_directory

        try:
            os.chdir(curr_path + "/" + dirname)
            curr_path = os.getcwd()
            curr_session.current_directory = curr_path
            conn.sendall(bytes("ok" + "\r\n", 'utf-8'))
        except FileNotFoundError as error:
            print(error)
            conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))

    elif comm == "ls":
        if len(args) != 0:
            dirname = args[0]
            try:
                print(os.listdir(curr_session.current_directory + "/" + dirname))
                conn.sendall(bytes(str(os.listdir(curr_session.current_directory + "/" + dirname)) + "\r\n", 'utf-8'))
            except FileNotFoundError as error:
                print(error)
                conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))
        else:
            print(os.listdir(curr_session.current_directory))
            conn.sendall(bytes(str(os.listdir(curr_session.current_directory)) + "\r\n", 'utf-8'))

    elif comm == "pwd":
        dirpath = curr_session.current_directory
        print(dirpath)
        conn.sendall(bytes(dirpath + "\r\n", 'utf-8'))

    # Directory manipulation
    elif comm == "mkdir":
        dirname = args[0]
        try:
            os.mkdir(curr_session.current_directory + "/" + dirname)
            conn.sendall(bytes("ok" + "\r\n", 'utf-8'))
        except OSError as error:
            print(error)
            conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))

    elif comm == "rmdir":
        dirname = args[0]

        try:
            shutil.rmtree(curr_session.current_directory + "/" + dirname)
            conn.sendall(bytes("ok" + "\r\n", 'utf-8'))
        except OSError as error:
            print(error)
            conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))

    # File Handling
    elif comm == "get":
        # CHECK IF THE FILE CLIENT WANTS EXISTS
        if check_if_file_already_exists(conn, args[0]):

            feedback = receive_message(conn)

            if feedback == "can get":
                filename = args[0]
                file = open(curr_session.current_directory + "/" + filename, "rb")
                aux = file.read(1024)
                while aux:
                    conn.send(aux)
                    aux = file.read(1024)
                conn.sendall(bytes("\r\n", 'utf-8'))
                print("enviou")

            # print("mandei ok")
            # conn.sendall(bytes("ok\r\n", 'utf-8'))
            # print("ok mandado")
        # except FileNotFoundError as error:
        #     print(error)
        #     conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))

    elif comm == "put":
        try:
            filename = args[0]
            check_if_file_already_exists(conn, filename)
            can_continue = receive_message(conn)
            if can_continue == "Y":
                f = open(curr_session.current_directory + "/" + filename, 'wb')
                aux = conn.recv(1024)
                while aux:
                    f.write(aux)
                    aux = conn.recv(1024)
                    if aux.endswith(bytes("\r\n", 'utf-8')):
                        break
                print("recebeu")
            conn.sendall(bytes("ok\r\n", 'utf-8'))
        except FileNotFoundError as error:
            print(error)
            conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))

    elif comm == "delete":
        filename = args[0]
        try:
            os.remove(curr_session.current_directory + "/" + filename)
            conn.sendall(bytes("ok" + "\r\n", 'utf-8'))
        except OSError as error:
            print(error)
            conn.sendall(bytes(str(error) + "\r\n", 'utf-8'))


def control_connection(conn, client_addr):
    print("control connection from" + str(client_addr))

    is_correct_authentication, username = check_authentication(conn)
    conn.sendall(bytes(str(is_correct_authentication) + "\r\n", 'utf-8'))
    if is_correct_authentication:
        print("[" + username + "] connected!")
        current_session = Session(os.getcwd(), username)
        sessions[conn] = current_session

        while True:
            command_line = receive_message(conn)
            command = command_line.split(" ")[0]
            command_args = command_line.split(" ")[1:]

            if command == "close" or command == "quit":
                conn.close()
                break
            else:
                manage_command_line(command, command_args, conn)
    else:
        print("[" + username + "] failed on authentication")
        conn.close()


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
    sock.listen(10)

    while True:
        # Wait for a connection
        connection, client_address = sock.accept()
        _thread.start_new_thread(control_connection, (connection, client_address))
