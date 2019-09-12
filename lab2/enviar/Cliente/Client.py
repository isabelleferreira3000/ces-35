import socket
import os


def get_command_line():
    line_input = input(">> ")
    comm = line_input.split(" ")[0]
    args = line_input.split(" ")[1:]
    return comm, args, line_input


def is_a_valid_command_line(comm, args):
    n_args = len(args)

    if comm == "cd" \
            or comm == "mkdir" or comm == "rmdir" \
            or comm == "get" or comm == "put" or comm == "delete" \
            or comm == "open":
        if n_args != 1:
            print("Error: 1 argument is expected to '" + comm + "'")
        return n_args == 1

    elif comm == "pwd" or comm == "close" or comm == "quit":
        if n_args != 0:
            print("Error: No argument is expected to '" + comm + "'")
        return n_args == 0

    elif comm == "ls":
        if n_args != 0 and n_args != 1:
            print("Error: 0 or 1 argument is expected to '" + comm + "'")
        return n_args == 0 or n_args == 1

    else:
        print("Error: Invalid command")
        return False


def receive_message(conn):
    message_received = ""
    while True:
        data_received = conn.recv(16)
        message_received = message_received + data_received.decode("utf-8")
        if message_received.endswith("\r\n"):
            break
    message_received = message_received.split("\r\n")[0]
    return message_received


def handle_feedback(comm, text):

    if comm == "cd" or \
            comm == "mkdir" or comm == "rmdir" or \
            comm == "get" or comm == "put" or comm == "delete":
        if text != "ok":
            print(text)

    elif comm == "ls":
        try:
            aux = text.split("[")[1:]
            aux = aux[0].split("]")[0]
            aux = aux.split(", ")
            for x in aux:
                x = x.split("'")[1]
                print(x)
        except:
            print(text)

    elif comm == "pwd":
        print(text)


def check_if_file_already_exists(path_filename):
    name = path_filename.rsplit('/', 1)[-1]
    path = ""
    if len(path_filename.rsplit('/', 1)) == 2:
        path = path_filename.rsplit('/', 1)[0]
        if path.startswith("/"):
            path = path[1:]

    files = os.listdir(os.getcwd() + "/" + path)
    if name in files:
        return True
    else:
        return False


if __name__ == "__main__":

    finished = False

    while not finished:

        command, command_args, _ = get_command_line()
        if is_a_valid_command_line(command, command_args):

            if command == "open":
                server = command_args[0]

                if server == "localhost:2121":
                    # Create a TCP/IP socket and connect the socket to the port where the server is listening
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    server_address = ('localhost', 2121)
                    sock.connect(server_address)

                    # authentication
                    username = input(receive_message(sock) + " ")
                    sock.sendall(bytes(username + "\r\n", 'utf-8'))
                    password = input(receive_message(sock) + " ")
                    sock.sendall(bytes(password + "\r\n", 'utf-8'))

                    authentication = receive_message(sock)
                    if authentication == "True":
                        print("Connected!")

                        while True:
                            command, command_args, command_line = get_command_line()
                            if is_a_valid_command_line(command, command_args):

                                command_line = command_line + "\r\n"

                                if command == "open":
                                    print("Error: already connected")

                                else:
                                    sock.sendall(bytes(command_line, 'utf-8'))

                                if command == "close":
                                    sock.close()
                                    break
                                elif command == "quit":
                                    sock.close()
                                    finished = True
                                    break

                                elif command == "get":
                                    # CHECK IF THE FILE CLIENT WANTS EXISTS IN SERVER
                                    feedback = receive_message(sock)

                                    if feedback == "file already exists":
                                        filename = command_args[0]

                                        # CHECK IF THE FILE CLIENT WANTS EXISTS IN LOCAL
                                        if len(filename.rsplit('/', 1)) == 2:
                                            filename = filename.rsplit('/', 1)[1]
                                        already_exists = check_if_file_already_exists(filename)

                                        can_get = True
                                        if already_exists:
                                            print("File already exists. Do you want to overwrite local file? [Y/N]")
                                            while True:
                                                answer = input()
                                                if answer.upper() == "Y":
                                                    break
                                                elif answer.upper() == "N":
                                                    can_get = False
                                                    break
                                                else:
                                                    print("Invalid answer. Please, answer with Y or N.")

                                        if can_get:
                                            sock.sendall(bytes("can get\r\n", 'utf-8'))
                                            f = open(str(filename), 'wb')
                                            aux = sock.recv(1024)
                                            while aux:
                                                f.write(aux)
                                                aux = sock.recv(1024)
                                                if aux.endswith(bytes("\r\n", 'utf-8')):
                                                    break
                                            print("File downloaded!")
                                        else:
                                            sock.sendall(bytes("can not get\r\n", 'utf-8'))

                                    else:
                                        print("Error: file does not exists in server")

                                elif command == "put":
                                    filename = command_args[0]

                                    # CHECK IF THE FILE CLIENT WANTS EXISTS IN LOCAL
                                    file_exists_in_local = check_if_file_already_exists(filename)

                                    if file_exists_in_local:
                                        sock.sendall(bytes("files exists in local\r\n", 'utf-8'))

                                        feedback = receive_message(sock)
                                        can_receive = False

                                        if feedback == "file already exists":
                                            print("File already exists. Do you want to overwrite remote file? [Y/N]")
                                            while True:
                                                answer = input()
                                                if answer.upper() == "Y":
                                                    sock.sendall(bytes("Y\r\n", 'utf-8'))
                                                    can_receive = True
                                                    break
                                                elif answer.upper() == "N":
                                                    sock.sendall(bytes("N\r\n", 'utf-8'))
                                                    break
                                                else:
                                                    print("Invalid answer. Please, answer with Y or N.")
                                        else:
                                            can_receive = True
                                            sock.sendall(bytes("Y\r\n", 'utf-8'))

                                        if can_receive:
                                            filename = filename.rsplit('/', 1)[-1]
                                            filepath = ""
                                            if len(filename.rsplit('/', 1)) == 2:
                                                filepath = filename.rsplit('/', 1)[0]
                                                if filepath.startswith("/"):
                                                    filepath = filepath[1:]

                                            curr_dir = os.getcwd()
                                            os.chdir(curr_dir + "/" + filepath)

                                            file = open(filename, "rb")
                                            aux = file.read(1024)
                                            while aux:
                                                sock.send(aux)
                                                aux = file.read(1024)
                                            sock.sendall(bytes("\r\n", 'utf-8'))

                                            os.chdir(curr_dir)
                                            print("File sent!")

                                        feedback = receive_message(sock)
                                        handle_feedback(command, feedback)

                                    else:
                                        print("Error: file does not exists in local")
                                        sock.sendall(bytes("files does not exists in local\r\n", 'utf-8'))

                                elif command != "open":
                                    feedback = receive_message(sock)
                                    handle_feedback(command, feedback)

                    else:
                        print("Authentication error: incorrect username or password")
                        sock.close()

                else:
                    print("Error: server not found. Try 'open localhost:2121'")

            elif command == "quit":
                finished = True
            else:
                print("Error: not connected!")
