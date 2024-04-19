CC = gcc
CFLAGS = -g
TARGET = build/clox.o
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=build/%.o)

.PHONY: all clean run

all: build_dir $(TARGET)

build_dir:
	mkdir -p build

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: %.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@ 

run:
	$(TARGET) $(path)

example:
	$(TARGET) ./examples/test.lox

clean:
	rm -rf build

