import os
import socket


def get_command_line():
    line_input = input()
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
        if data_received.decode("utf-8").endswith("\r\n"):
            break
    message_received = message_received.split("\r\n")[0]
    return message_received


def handle_feedback(comm, text):

    if comm == "cd":
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

    # Directory manipulation
    elif comm == "mkdir":
        if text != "ok":
            print(text)

    elif comm == "rmdir":
        if text != "ok":
            print(text)

    # File Handling
    elif comm == "get":
        if text != "ok":
            print(text)

    elif comm == "put":
        if text != "ok":
            print(text)

    elif comm == "delete":
        if text != "ok":
            print(text)


if __name__ == "__main__":

    finished = False

    while not finished:

        command, command_args, _ = get_command_line()
        if is_a_valid_command_line(command, command_args):

            if command == "open":
                # Create a TCP/IP socket and connect the socket to the port where the server is listening
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                server_address = ('localhost', 2122)
                sock.connect(server_address)

                # authentication
                print(receive_message(sock))
                username = input()
                sock.sendall(bytes(username + "\r\n", 'utf-8'))
                print(receive_message(sock))
                password = input()
                sock.sendall(bytes(password + "\r\n", 'utf-8'))

                authentication = receive_message(sock)
                if authentication == "True":
                    print("connected")

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
                            else:
                                feedback = receive_message(sock)
                                handle_feedback(command, feedback)

                else:
                    print("Authentication error: incorrect username or password")
                    sock.close()

            elif command == "quit":
                finished = True
            else:
                print("not connected")
                pass
