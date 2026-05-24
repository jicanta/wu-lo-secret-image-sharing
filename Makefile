CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g
TARGET  = visualSIS
SRCDIR  = src
SRCS    = $(SRCDIR)/main.c $(SRCDIR)/args.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
