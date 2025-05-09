FROM ubuntu:22.04

# Setting environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Installing basic tools and Qt
RUN apt-get update && apt-get install -y \
    build-essential \
    libgl1-mesa-dev \
    libgl1-mesa-glx \
    libx11-dev \
    cmake \
    git \
    vim \
    nano \
    wget \
    curl \
    sudo \
    htop \
    pkg-config \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qtcreator \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    gdb \
    clang \
    valgrind \
    ninja-build \
    ccache \
    openssh-server \
    x11-apps \
    x11-utils \
    mesa-utils \
    dbus-x11 \
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
RUN echo 'export PS1="\[\e[1;32m\][\[\e[m\]\[\e[1;34m\]Root\[\e[m\]\[\e[1;32m\]]\[\e[m\] \[\e[1;36m\]\w\[\e[m\] \[\e[1;32m\]\\$\[\e[m\] "' >> /root/.bashrc && \
    echo "alias ll='ls -la'" >> /root/.bashrc && \
    echo "alias cls='clear'" >> /root/.bashrc && \
    echo "echo ''" >> /root/.bashrc && \
    echo "echo '  C++/Qt Development Environment'" >> /root/.bashrc && \
    echo "echo '  ----------------------------'" >> /root/.bashrc && \
    echo "echo ''" >> /root/.bashrc 

# Expose SSH port
EXPOSE 22

# Start SSH server and bash
CMD service ssh start && /bin/bash
