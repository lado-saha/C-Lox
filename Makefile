# CC = gcc
# CFLAGS = -Wall -g
#

SOURCES = main.c chunk.c memory.c 
# OBJECTS = $(SOURCES:.c = .o)
#
# all: 
# 	$(SOURCES)
#
# $(SOURCES): $(SOURCES)
#
# %.o : %.c
# 	$(CC) $(CFLAGS) -c $< -o $@



$(SOURCES):
	touch $(SOURCES)


