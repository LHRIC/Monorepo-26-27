import socket


def main():
    with socket.create_connection(("127.0.0.1", 2222), timeout=3) as connection:
        if not connection.recv(256).startswith(b"SSH-2.0-"):
            raise SystemExit(1)


if __name__ == "__main__":
    main()
