import os
import socket


if __name__ == "__main__":
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    server_address = ('localhost', 2122)
    print('connecting to {} port {}'.format(*server_address))
    sock.connect(server_address)

    while True:
        command_line = input()

        # Send data
        print("enviando: " + str(command_line))
        sock.sendall(bytes(command_line, 'utf-8'))

        # # Create a TCP/IP socket
        # sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #
        # # Connect the socket to the port where the server is listening
        # server_address = ('localhost', 2122)
        # print('connecting to {} port {}'.format(*server_address))
        # sock.connect(server_address)

        # if is_a_valid_command_line(command_line):
        #     command_args = command_line.split(" ")
        #     command = command_args[0]
        #
        #     # Directory Browsing and Listing
        #     if command == "cd":
        #         dirname = command_args[1]
        #         print(dirname)
        #
        #     elif command == "ls":
        #         if len(command_args) != 1:
        #             dirname = command_args[1]
        #             print(dirname)
        #
        #     elif command == "pwd":
        #         pass
        #
        #     # Directory manipulation
        #     elif command == "mkdir":
        #         dirname = command_args[1]
        #         print(dirname)
        #
        #     elif command == "rmdir":
        #         dirname = command_args[1]
        #         print(dirname)
        #
        #     # File Handling
        #     elif command == "get":
        #         filename = command_args[1]
        #         print(filename)
        #     elif command == "put":
        #         filename = command_args[1]
        #         print(filename)
        #     elif command == "delete":
        #         filename = command_args[1]
        #         print(filename)
        #
        #     # Connection Management
        #     elif command == "close":
        #         pass
        #     elif command == "open":
        #         server = command_args[1]
        #         # if server == "server.ita.br":
        #         if True:
        #             # Create a TCP/IP socket
        #             sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        #             # Connect the socket to the port where the server is listening
        #             server_address = ('localhost', 2122)
        #             print('connecting to {} port {}'.format(*server_address))
        #             sock.connect(server_address)
        #
        #             # Receive the data in small chunks and retransmit it
        #             while True:
        #                 data = sock.recv(16)
        #                 if data:
        #                     print(data)
        #                 else:
        #                     print('no data from', client_address)
        #                     break
        #
        #             user = input()
        #             sock.sendall(bytes(user, 'utf-8'))
        #
        #             # Receive the data in small chunks and retransmit it
        #             while True:
        #                 data = sock.recv(16)
        #                 if data:
        #                     print(data)
        #                 else:
        #                     print('no data from', client_address)
        #                     break
        #
        #             password = input()
        #             sock.sendall(bytes(password, 'utf-8'))
        #
        #         else:
        #             print("No server " + server)
        #
        #     elif command == "quit":
        #         break
