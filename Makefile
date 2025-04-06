# Makefile for persistent C++/Qt environment

# Configuration variables
CONTAINER_NAME = cpp-qt-dev
IMAGE_NAME = my-cpp-qt-dev
# Fix for Windows/MSYS path issues
WORKSPACE_DIR = $(subst \,/,$(CURDIR))/workspace
HOME_VOLUME_NAME = cpp-qt-home-volume

# Add this for all Docker commands to prevent path conversion in MSYS
DOCKER = MSYS_NO_PATHCONV=1 docker

.PHONY: build start stop connect status clean clean-all rebuild

# Building Docker image
build:
	@echo "Building image..."
	@mkdir -p $(WORKSPACE_DIR)
	@$(DOCKER) build -t $(IMAGE_NAME) .

# Starting and configuring X-server
start:
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
			-e DISPLAY=$(DISPLAY) \
			-p 2222:22 \
			--network host \
			-w /workspace \
			$(IMAGE_NAME); \
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