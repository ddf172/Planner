FROM ubuntu:22.04

# Setting environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Installing basic tools and required libraries
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    vim \
    nano \
    wget \
    curl \
    sudo \
    htop \
    pkg-config \
    libboost-all-dev \
    gdb \
    clang \
    valgrind \
    ninja-build \
    ccache \
    openssh-server \
    net-tools \
    iputils-ping \
    netcat \
    && rm -rf /var/lib/apt/lists/*

# Configure SSH for remote development
RUN mkdir -p /run/sshd && \
    echo "root:root" | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# Creating workspace directory
RUN mkdir -p /workspace

# Setting working directory
WORKDIR /workspace

# Configuring Bash environment
RUN echo 'export PS1="\[\e[1;31m\][\[\e[m\]\[\e[1;34m\]Server\[\e[m\]\[\e[1;31m\]]\[\e[m\] \[\e[1;36m\]\w\[\e[m\] \[\e[1;31m\]\\$\[\e[m\] "' >> /root/.bashrc && \
    echo "alias ll='ls -la'" >> /root/.bashrc && \
    echo "alias cls='clear'" >> /root/.bashrc && \
    echo "echo ''" >> /root/.bashrc && \
    echo "echo '  C++ Server Environment'" >> /root/.bashrc && \
    echo "echo '  ----------------------'" >> /root/.bashrc && \
    echo "echo ''" >> /root/.bashrc 

# Expose SSH port and application server port
EXPOSE 22
EXPOSE 8080

# Start SSH server and bash
CMD service ssh start && /bin/bash
