CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g
TARGET  = visualSIS
SRCDIR  = src
TESTDIR = tests
SRCS    = $(SRCDIR)/main.c $(SRCDIR)/args.c $(SRCDIR)/poly.c $(SRCDIR)/bmp.c $(SRCDIR)/dir.c $(SRCDIR)/prng.c
OBJS    = $(SRCS:.c=.o)

TEST_BINS = $(TESTDIR)/test_poly $(TESTDIR)/test_bmp

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

$(TESTDIR)/test_bmp: $(TESTDIR)/test_bmp.o $(SRCDIR)/bmp.o
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do ./$$t; done

clean:
	rm -f $(OBJS) $(SRCDIR)/poly.o $(SRCDIR)/bmp.o $(TARGET) $(TEST_BINS) $(TESTDIR)/*.o
