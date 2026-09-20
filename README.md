# SockChat

A lightweight, multi-client **TCP chat application written in C** using raw POSIX sockets and `pthreads`. One server relays messages between any number of connected terminal clients, with no external dependencies.

SockChat is a hands-on learning project for network programming fundamentals: creating sockets, binding and listening, accepting connections, threading, and broadcasting data between peers.

---

## Table of Contents

- [Features](#features)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Configuration](#configuration)
- [Module Reference](#module-reference)
- [Protocol](#protocol)
- [Known Limitations](#known-limitations)
- [Roadmap](#roadmap)
- [Contributing](#contributing)

---

## Features

- **Multi-client chat room**: every message a client sends is broadcast to all *other* connected clients.
- **Thread-per-connection server**: each accepted client gets a dedicated receiver thread, so clients never block each other.
- **Concurrent send/receive on the client**: a background thread prints incoming messages while the main thread reads your input.
- **Display names**: choose a username at startup; messages are sent as `name:message`.
- **Shared utility library**: socket creation and address helpers live in a reusable static library (`SocketUtil`).
- **Zero dependencies**: only the C standard library, POSIX sockets, and pthreads.

---

## How It Works

```
   ┌──────────┐                                   ┌──────────┐
   │ Client A │──── "Omar:hello" ───────┐        │ Client B │
   └──────────┘                          ▼        └──────────┘
                                   ┌────────────┐       ▲
   ┌──────────┐                    │   Server   │       │
   │ Client C │◄── "Omar:hello" ──│ (port 8080)│───────┘
   └──────────┘                    └────────────┘
                              broadcasts to everyone
                              except the sender
```

1. The **server** creates a TCP socket, binds it to port `8080` on all interfaces, and listens.
2. For each incoming connection it stores the client's socket in a table and spawns a **receiver thread**.
3. When a receiver thread gets data, it prints it on the server console and **forwards it to every other client**.
4. The **client** connects, asks for a name, then runs two concurrent loops: a **listener thread** that prints incoming messages, and a **main loop** that reads from stdin and sends `name:message`.
5. Typing `exit` closes the client connection.

---

## Project Structure

```
SockChat/
├── SocketUtil/          # Shared static library
│   ├── socketutil.h     # Public API + accepted_socket struct
│   ├── socketutil.c     # Socket & address helpers
│   └── CMakeLists.txt
├── SocketServer/        # Chat server
│   ├── main.c           # Accept loop, per-client threads, broadcast
│   └── CMakeLists.txt
└── SocketClient/        # Terminal chat client
    ├── main.c           # Connect, name prompt, send/receive loops
    └── CMakeLists.txt
```

Each folder is an independent CMake project (originally developed in CLion under WSL). The server and client both link against `SocketUtil`.

---

## Getting Started

### Prerequisites

- A **POSIX environment**: Linux, macOS, or **WSL** on Windows (the code uses `<sys/socket.h>`, `<arpa/inet.h>`, and `<pthread.h>`, so it will not compile natively with MSVC or MinGW)
- **GCC** or **Clang** (C11 support)
- *(Optional)* **CMake 3.20+**

### Build with CMake

The bundled `CMakeLists.txt` files for the server and client locate `SocketUtil` using a **hard-coded absolute path** from the My machine:

```cmake
find_path(TheHeaderFile socketutil.h
        PATHS /mnt/c/Users/Omar/Desktop/SockChat/SocketUtil)
find_library(TheLibrary NAMES SocketUtil
        PATHS /mnt/c/Users/Omar/Desktop/SockChat/SocketUtil/cmake-build-debug-wsl)
```

To build with CMake on your machine, first build the library, then edit those two `PATHS` entries to point at your local checkout:

```bash
# 1. Build the static library
cd SocketUtil
cmake -S . -B build && cmake --build build     # produces build/libSocketUtil.a

# 2. Update the PATHS in SocketServer/CMakeLists.txt and SocketClient/CMakeLists.txt
#    (header dir: SocketUtil/, library dir: SocketUtil/build)

# 3. Build the server and client
cd ../SocketServer && cmake -S . -B build && cmake --build build
cd ../SocketClient && cmake -S . -B build && cmake --build build
```

---

## Usage

Open **at least two terminals** (three or more makes it more fun).

**Terminal 1: start the server**

```bash
./sockchat-server
```

```
socket bound successfully
socket listen successfully
Waiting for client...
```

**Terminals 2, 3, …: start clients**

```bash
./sockchat-client
```

```
Connection established successfully
what should we call you?:
 alice
type a message (type exit to escape):
 hello everyone!
```

Other clients will see:

```
alice:hello everyone!
```

Type `exit` to leave the chat.

---

## Configuration

Settings are currently compile-time constants:

| Setting | Value | Location |
| --- | --- | --- |
| Server listen port | `8080` | `SocketServer/main.c` (`createIPv4Address("", 8080)`) |
| Server bind address | All interfaces (`INADDR_ANY`) | `SocketServer/main.c` |
| Client target address | `127.0.0.1:8080` | `SocketClient/main.c` |
| Max tracked clients | `10` | `accepted_sockets[10]` in `SocketServer/main.c` |
| Listen backlog | `10` | `listen(server_socket_fd, 10)` |
| Message buffer size | `1024` bytes | Both `main.c` files |

To chat across machines, change the client's IP `127.0.0.1` to the server's IP address and rebuild (and make sure port `8080` is reachable through any firewall).

---

## Module Reference

### `SocketUtil`

| Function | Description |
| --- | --- |
| `int createTCPIpv4Socket()` | Creates an `AF_INET` / `SOCK_STREAM` socket and returns its file descriptor. |
| `struct sockaddr_in* createIPv4Address(char *ip, int port)` | Allocates and fills an IPv4 address. An empty `ip` string means "any interface". The caller owns the returned heap memory. |
| `struct accepted_socket` | Bundles a client's file descriptor, address, error code, and an `acceptedSuccessfully` flag. |

The header also declares `receiveAndPrintIncomingDataOnSeparateThread` and `sendReceiveMessageToTheOtherClients`; these are implemented in the **server** (`SocketServer/main.c`), not in the library.

### `SocketServer`

| Function | Description |
| --- | --- |
| `acceptIncomingConnection` | Blocks on `accept()` and returns a heap-allocated `accepted_socket`. |
| `startAcceptingIncomingConnections` | Infinite accept loop; records each client and starts its thread. |
| `receiveAndPrintIncomingDataOnSeparateThread` | Spawns a `pthread` for one client. |
| `receiveAndPrintIncomingData` | Per-client loop: `recv()` → print → broadcast; exits on disconnect. |
| `sendReceiveMessageToTheOtherClients` | Sends a message to every client except the sender. |

### `SocketClient`

| Function | Description |
| --- | --- |
| `startListeningAndPrintMessagesOnNewThread` | Starts the background listener thread. |
| `listenAndPrint` | Receives and prints messages from the server until it closes. |
| `main` | Connects, prompts for a name, and loops reading stdin and sending `name:message`. |

---

## Protocol

SockChat uses a deliberately simple, **plain-text protocol over TCP**:

- There is no handshake or framing: each `send()` carries one chat message.
- Client → server payload: `<name>:<message>` (no trailing newline, no terminator)
- Server → other clients: the same bytes, unmodified
- A client disconnects by closing its socket (`exit` is handled locally and is *not* sent)


## Roadmap for Improvements

- [ ] Remove clients from the table on disconnect
- [ ] Protect shared state with a mutex; use a dynamic client list
- [ ] Length-prefixed or newline-delimited message framing
- [ ] Configurable host/port via command-line arguments or env vars
- [ ] Notify others on join / leave; announce the `exit` command
- [ ] Portable CMake setup (`add_subdirectory` + a top-level `CMakeLists.txt`)
- [ ] Add a `.gitignore` for `cmake-build-*` and `.idea/`
- [ ] Private messages and user list commands
- [ ] TLS support
