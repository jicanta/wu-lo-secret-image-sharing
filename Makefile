CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g
TARGET  = visualSIS
SRCDIR  = src
TESTDIR = tests
SRCS    = $(SRCDIR)/main.c $(SRCDIR)/args.c
OBJS    = $(SRCS:.c=.o)

TEST_BINS = $(TESTDIR)/test_poly

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TESTDIR)/%.o: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) -I$(SRCDIR) -c -o $@ $<

$(TESTDIR)/test_poly: $(TESTDIR)/test_poly.o $(SRCDIR)/poly.o
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do ./$$t; done

clean:
	rm -f $(OBJS) $(SRCDIR)/poly.o $(TARGET) $(TEST_BINS) $(TESTDIR)/*.o
