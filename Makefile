# Makefile for persistent C++/Qt environment

# Configuration variables
CONTAINER_NAME = cpp-qt-dev
IMAGE_NAME = my-cpp-qt-dev
HOME_VOLUME_NAME = cpp-qt-home-volume
WORKSPACE_DIR = $(subst \,/,$(CURDIR))/workspace

# Default docker command
DOCKER = docker

.PHONY: build start-wsl start-linux start-mac stop connect status clean clean-all rebuild

# Building Docker image
build:
	@echo "Building image..."
	@mkdir -p $(WORKSPACE_DIR)
	@$(DOCKER) build -t $(IMAGE_NAME) .

# Starting container in WSL environment
start-wsl:
	@echo "Starting in WSL environment..."
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Container is already running. Use 'make connect'."; \
	elif [ "$$($(DOCKER) ps -aq -f name=$(CONTAINER_NAME))" ]; then \
		echo "Resuming stopped container..."; \
		$(DOCKER) start $(CONTAINER_NAME); \
		$(DOCKER) exec -it $(CONTAINER_NAME) bash -c "export DISPLAY=:0 && /bin/bash"; \
	else \
		echo "Creating new container..."; \
		$(DOCKER) volume create $(HOME_VOLUME_NAME); \
		$(DOCKER) run -it \
			--name $(CONTAINER_NAME) \
			-v $(WORKSPACE_DIR):/workspace \
			-v $(HOME_VOLUME_NAME):/root \
			-v /tmp/.X11-unix:/tmp/.X11-unix \
			-v /mnt/wslg:/mnt/wslg \
			-e DISPLAY=:0 \
			-e WAYLAND_DISPLAY \
			-e XDG_RUNTIME_DIR \
			-e PULSE_SERVER \
			-p 2222:22 \
			--network host \
			--security-opt apparmor:unconfined \
			-w /workspace \
			$(IMAGE_NAME) bash -c "export DISPLAY=:0 && /bin/bash"; \
	fi

# Starting container in Linux environment
start-linux:
	@echo "Starting in Linux environment..."
	@xhost +local:docker > /dev/null 2>&1 || true
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Container is already running. Use 'make connect'."; \
	elif [ "$$($(DOCKER) ps -aq -f name=$(CONTAINER_NAME))" ]; then \
		echo "Resuming stopped container..."; \
		$(DOCKER) start $(CONTAINER_NAME); \
		$(DOCKER) exec -it $(CONTAINER_NAME) /bin/bash; \
	else \
		echo "Creating new container..."; \
		$(DOCKER) volume create $(HOME_VOLUME_NAME); \
		$(DOCKER) run -it \
			--name $(CONTAINER_NAME) \
			-v $(WORKSPACE_DIR):/workspace \
			-v $(HOME_VOLUME_NAME):/root \
			-v /tmp/.X11-unix:/tmp/.X11-unix \
			-e DISPLAY=$(DISPLAY) \
			-p 2222:22 \
			--network host \
			--privileged \
			-w /workspace \
			$(IMAGE_NAME); \
	fi

# Starting container in macOS environment
start-mac:
	@echo "Starting in macOS environment..."
	@echo "Make sure XQuartz is running and allows network clients"
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Container is already running. Use 'make connect'."; \
	elif [ "$$($(DOCKER) ps -aq -f name=$(CONTAINER_NAME))" ]; then \
		echo "Resuming stopped container..."; \
		$(DOCKER) start $(CONTAINER_NAME); \
		$(DOCKER) exec -it $(CONTAINER_NAME) /bin/bash; \
	else \
		echo "Creating new container..."; \
		$(DOCKER) volume create $(HOME_VOLUME_NAME); \
		$(DOCKER) run -it \
			--name $(CONTAINER_NAME) \
			-v $(WORKSPACE_DIR):/workspace \
			-v $(HOME_VOLUME_NAME):/root \
			-e DISPLAY=host.docker.internal:0 \
			-e LIBGL_ALWAYS_INDIRECT=1 \
			-p 2222:22 \
			-w /workspace \
			$(IMAGE_NAME); \
	fi

# General start (tries to auto-detect system)
start:
	@if [ -d /mnt/c/Windows ]; then \
		echo "Detected WSL, redirecting to start-wsl..."; \
		$(MAKE) start-wsl; \
	elif [ "$$(uname)" = "Darwin" ]; then \
		echo "Detected macOS, redirecting to start-mac..."; \
		$(MAKE) start-mac; \
	else \
		echo "Detected Linux, redirecting to start-linux..."; \
		$(MAKE) start-linux; \
	fi

# Stopping container
stop:
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Stopping container..."; \
		$(DOCKER) stop $(CONTAINER_NAME); \
	else \
		echo "Container is not running."; \
	fi

# Connecting to running container
connect:
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Connecting to container..."; \
		$(DOCKER) exec -it $(CONTAINER_NAME) /bin/bash; \
	else \
		echo "Container is not running. Use 'make start'."; \
	fi

# Checking status
status:
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Container is running."; \
	elif [ "$$($(DOCKER) ps -aq -f name=$(CONTAINER_NAME))" ]; then \
		echo "Container exists but is stopped."; \
	else \
		echo "Container does not exist."; \
	fi
	@if [ "$$($(DOCKER) volume ls -q -f name=$(HOME_VOLUME_NAME))" ]; then \
		echo "Home volume exists."; \
	else \
		echo "Home volume does not exist."; \
	fi

# Removing container (without removing volume)
clean:
	@if [ "$$($(DOCKER) ps -q -f name=$(CONTAINER_NAME))" ]; then \
		echo "Stopping container..."; \
		$(DOCKER) stop $(CONTAINER_NAME); \
	fi
	@if [ "$$($(DOCKER) ps -aq -f name=$(CONTAINER_NAME))" ]; then \
		echo "Removing container..."; \
		$(DOCKER) rm $(CONTAINER_NAME); \
	fi
	@echo "Data in volume remains preserved."

# Complete cleanup (container and volume)
clean-all: clean
	@if [ "$$($(DOCKER) volume ls -q -f name=$(HOME_VOLUME_NAME))" ]; then \
		echo "Removing volume..."; \
		$(DOCKER) volume rm $(HOME_VOLUME_NAME); \
	fi
	@echo "All data has been removed."

# Rebuilding environment
rebuild: clean build start
