CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
LIBS = -lpthread

COMMON_SRC = common.c
COMMON_OBJ = common.o

SERVER_SRC = server.c
SERVER_OBJ = server.o
SERVER_TARGET = server

CLIENT_SRC = client.c
CLIENT_OBJ = client.o
CLIENT_TARGET = client

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(CLIENT_TARGET): $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(SERVER_OBJ): $(SERVER_SRC) common.h
	$(CC) $(CFLAGS) -c $<

$(CLIENT_OBJ): $(CLIENT_SRC) common.h
	$(CC) $(CFLAGS) -c $<

$(COMMON_OBJ): $(COMMON_SRC) common.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) *.o

distclean: clean
	rm -f *~