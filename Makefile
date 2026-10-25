CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_GNU_SOURCE -Iinclude -pthread
TARGET = sentinel-dispatcher

SRCS = src/main.c src/tcb.c src/queue.c src/parser.c src/scheduler.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean