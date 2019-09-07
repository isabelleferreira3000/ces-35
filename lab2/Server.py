import socket
import _thread
import sys


def is_a_valid_command_line(command_line):
    command_args = command_line.split(" ")
    command = command_args[0]
    n_args = len(command_args)

    # Directory Browsing and Listing
    if command == "cd" \
            or command == "mkdir" or command == "rmdir" \
            or command == "get" or command == "put" or command == "delete" \
            or command == "open":
        if n_args != 2:
            print("Error: 1 argument is expected")
        return n_args == 2

    elif command == "pwd" or command == "close" or command == "quit":
        if n_args != 1:
            print("Error: No argument is expected")
        return n_args == 1

    elif command == "ls":
        if n_args != 1 and n_args != 2:
            print("Error: 0 or 1 argument is expected")
        return n_args == 1 or n_args == 2

    else:
        print("Invalid command")
        return False


def test_thread(some_text):
    print("example: " + str(some_text))


if __name__ == "__main__":
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind the socket to the port
    server_address = ('localhost', 2122)
    print('starting up on {} port {}'.format(*server_address))
    sock.bind(server_address)

    # Listen for incoming connections
    sock.listen(1)

    while True:
        # Wait for a connection
        print('waiting for a connection')
        _thread.start_new(test_thread, ("oi",))
        connection, client_address = sock.accept()

        try:
            print('connection from', client_address)

            # Receive the data in small chunks and retransmit it
            message = ""
            i = 0
            while True:
                data = connection.recv(16)
                print("recebido: " + data.decode("utf-8"))
                message = message + data.decode("utf-8")
                if len(data.decode("utf-8")) < 16:
                    break
            print("mensagem: " + str(message))

        finally:
            # Clean up the connection
            connection.close()
