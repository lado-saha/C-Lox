CC = gcc
CFLAGS =  -g
TARGET = build/clox
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=build/%.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run:
	$(TARGET) $(path)

example:
	$(TARGET) ./examples/test.lox

clean:
	rm -f $(OBJS) $(TARGET)
