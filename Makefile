PROJECT = cargs
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -g
OBJS = \
	src/test.o \
	src/cargs.o

.PHONY: all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: $(PROJECT)

$(PROJECT): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -vf $(OBJS) $(PROJECT)
