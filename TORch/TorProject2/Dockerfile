FROM ubuntu:22.04

# Install build tools + OpenSSL
RUN apt update && apt install -y \
    g++ cmake build-essential libssl-dev

WORKDIR /app

# Copy source code
COPY . .

# Build the server
RUN mkdir build && cd build && cmake .. && make

# Command to run the server
CMD ["bash", "-c", "cd build/TorProject2 && ./TorProject2"]
