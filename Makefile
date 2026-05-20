CC = gcc
CFLAGS = -Wall -Werror -Iinclude
SRCS = src/main.c src/common.c src/parser.c src/builtin.c src/executor.c
OBJS = $(SRCS:.c=.o)
TARGET = shell

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)