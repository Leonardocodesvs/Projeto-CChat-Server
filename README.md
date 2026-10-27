# C Chat Server

A TCP chat server developed in C for Windows using Winsock2 and multithreading. The project allows multiple clients to connect to a server and exchange messages in real time.

This project was created for educational purposes, focusing on network programming, client-server communication, and concurrency in C.

## Features

- TCP client-server communication
- Multiple simultaneous client connections
- Multithreading using Windows threads
- Real-time message broadcasting
- `/users` command to list connected users
- `/quit` command to disconnect from the chat
- Python client for testing server communication

## Technologies

- **C** — Server implementation
- **Python** — Client implementation
- **Winsock2** — Windows network programming
- **TCP/IP** — Communication protocol
- **Windows Threads** — Concurrent client handling
- **Git and GitHub** — Version control and source code hosting

## Project Structure

```text
CChat-Server/
├── src/
│   ├── main.c
│   ├── server.c
│   ├── server.h
│   └── cliente.py
├── .gitignore
├── Makefile.win
└── README.md
```

*Adjust the file structure above to match your actual repository.*

## Requirements

- Windows
- GCC compiler with Winsock2 support
- Python 3, to run the Python client

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/YOUR-USERNAME/CChat-Server.git
cd CChat-Server
```

### 2. Compile the server

Compile the C source files using GCC:

```bash
gcc src/main.c src/server.c -o server.exe -lws2_32
```

If your project uses additional compiler options or a different folder structure, adjust the command accordingly.

### 3. Run the server

```bash
server.exe
```

### 4. Run the client

Open another terminal and run:

```bash
python src/cliente.py
```

Start additional client instances to test communication between multiple users.

## Configuration

The server uses the following local connection settings:

```python
HOST = "127.0.0.1"
PORT = 5555
```

The `127.0.0.1` address allows local connections only. The client and server must run on the same computer unless the server's network configuration is changed.

## Learning Goals

- Understanding TCP sockets and network communication
- Learning the client-server architecture
- Exploring multithreading in C
- Managing multiple client connections
- Practicing synchronization and resource management
- Using Python to test network applications

## Security and Limitations

This is an educational project and is not intended for production use.

The current version should not be considered secure for untrusted networks. Authentication, TLS encryption, connection limits, and additional concurrency and stress testing may be needed before broader deployment.

## Development

This project was developed with the assistance of AI tools as part of a practical learning process.

## License

No license has been specified yet. Unless a license is added, others may be able to view and clone the public repository, but they do not automatically receive permission to redistribute or reuse the code under an open-source license.
