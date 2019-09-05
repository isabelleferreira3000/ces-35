import os


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


if __name__ == "__main__":
    current_dir_path = os.getcwd()
    print("current directory is : " + current_dir_path)
    folder_name = os.path.basename(current_dir_path)
    print("Directory name is : " + folder_name)

    while True:
        command_line = input()

        if is_a_valid_command_line(command_line):
            command_args = command_line.split(" ")
            command = command_args[0]

            # Directory Browsing and Listing
            if command == "cd":
                dirname = command_args[1]
                print(dirname)

            elif command == "ls":
                if len(command_args) != 1:
                    dirname = command_args[1]
                    print(dirname)

            elif command == "pwd":
                print(current_dir_path)

            # Directory manipulation
            elif command == "mkdir":
                dirname = command_args[1]
                print(dirname)

            elif command == "rmdir":
                dirname = command_args[1]
                print(dirname)

            # File Handling
            elif command == "get":
                filename = command_args[1]
                print(filename)
            elif command == "put":
                filename = command_args[1]
                print(filename)
            elif command == "delete":
                filename = command_args[1]
                print(filename)

            # Connection Management
            elif command == "close":
                pass
            elif command == "open":
                filename = command_args[1]
                print(filename)
            elif command == "quit":
                break
