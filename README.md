# TOR Network Simulation

<div align="center">
  <a href="https://github.com/LavieNaor/TORCH.git">
    <img src="images/onion.png" alt="Logo" width="80" height="80">
  </a>
</div>

## Overview

This project implements a simplified TOR-like anonymous communication
network.

The system allows a client to communicate with a server through multiple
intermediate routers in order to hide the client's identity and simulate
onion routing.

The project was developed as part of a cybersecurity and networking
learning program.

------------------------------------------------------------------------

## System Components

### Client (C#)

The client application allows the user to send messages through the TOR
network.

Responsibilities: - Building the onion encrypted message - Sending the
message to the entry router - Receiving responses from the server

------------------------------------------------------------------------

### Routers (Python)

Three routers simulate TOR relays.

Each router: - Receives encrypted messages - Decrypts one layer of
encryption - Forwards the message to the next router

Routers do not know the full path of the message, which preserves
anonymity.

------------------------------------------------------------------------

### Server (C++)

The server receives the final decrypted message from the last router.

Responsibilities: - Processing client requests - Sending responses back
through the router chain

------------------------------------------------------------------------

### Docker Environment

Docker is used to run the server and routers in isolated containers,
allowing the system to run easily on any machine.

------------------------------------------------------------------------

## Technologies Used

-   C++
-   Python
-   C#
-   Docker
-   Docker Compose
-   WSL (Windows Subsystem for Linux)
-   Socket Programming
-   Encryption

------------------------------------------------------------------------

## Project Architecture

Client → Router 1 → Router 2 → Router 3 → Server

Each router removes one encryption layer before forwarding the message.

------------------------------------------------------------------------

## Installation

To run this project on a new machine, install the following tools:

### 1. Install Git

Used to clone the repository.

https://git-scm.com/

Verify installation:

``` bash
git --version
```

------------------------------------------------------------------------

### 2. Install Docker Desktop

Docker is used to run the server and routers.

https://www.docker.com/products/docker-desktop/

Verify installation:

``` bash
docker --version
```

------------------------------------------------------------------------

### 3. Install WSL

The server runs in a Linux environment.

Run in PowerShell:

``` powershell
wsl --install
```

------------------------------------------------------------------------

### 4. Install Visual Studio

Required to run the C# client.

Install Visual Studio with the following workload:

-   .NET Desktop Development

https://visualstudio.microsoft.com/

------------------------------------------------------------------------

### 5. Install Python (Optional)

Only required if you want to run the routers outside Docker.

https://www.python.org/

Verify installation:

``` bash
python --version
```

------------------------------------------------------------------------

## Clone the Repository

Clone the project:

``` bash
git clone https://github.com/<username>/<repository-name>.git
```

Enter the project folder:

``` bash
cd <repository-name>
```

------------------------------------------------------------------------

## Running the Project

### Step 1 -- Build Docker Containers

``` bash
docker compose build
```

------------------------------------------------------------------------

### Step 2 -- Start the Server and Routers

``` bash
docker compose up
```

This command will start: - The C++ server - Router 1 - Router 2 - Router
3

------------------------------------------------------------------------

### Step 3 -- Run the Client

1.  Open the Client folder.
2.  Open the `.sln` file in Visual Studio.
3.  Press Run.

The client will connect to the TOR network through the first router.

------------------------------------------------------------------------

### Step 4 -- Verify Containers

``` bash
docker ps
```

You should see containers for: - server - router1 - router2 - router3

------------------------------------------------------------------------

## Security Concept -- Onion Routing

The system simulates the concept of onion routing.

The client encrypts the message multiple times using the public keys of
the routers.

Each router removes one layer of encryption, revealing the address of
the next router.

This ensures that no single router knows both: - the origin of the
message - the final destination

------------------------------------------------------------------------

## Future Improvements

Possible improvements include:

- Add sending files feature
- Add voice call and video call feature
- Add sending voice messages feature

------------------------------------------------------------------------

## Authors

Lavie Naor
