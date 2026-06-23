CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I/tmp/raylib/src
LDFLAGS = -L/usr/lib64 -l:libraylib.so.550 -lm

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = game

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
