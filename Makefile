# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

# Directories
SRC_DIR = src
INC_DIR = includes

# Server files
SERVER_SRCS = $(SRC_DIR)/server.c $(SRC_DIR)/file_operations.c \
              $(SRC_DIR)/admin.c $(SRC_DIR)/faculty.c $(SRC_DIR)/student.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)

# Client files
CLIENT_SRCS = $(SRC_DIR)/client.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# Executables
SERVER_TARGET = academia_server
CLIENT_TARGET = academia_client

# Header files
INCLUDES = -I$(INC_DIR)

.PHONY: all clean server client

all: server client

server: $(SERVER_TARGET)

client: $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET)

run_server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run_client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)