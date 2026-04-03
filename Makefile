CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O3
LDFLAGS = -lgdi32 -luser32

SRC = main.c renderer.c
OBJ = $(SRC:.c=.o)
TARGET = subpixel_renderer.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(OBJ) $(TARGET)
