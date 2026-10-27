CFLAGS = -std=c11
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O2
LDLIBS = -pthread

SRC = src/main.c src/server.c
TARGET = cchat-server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) src/server.h
	$(CC) $(CFLAGS) -Isrc $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)
