
INSTALL = /usr/bin/install -p
CD = /usr/bin/cd
TEST_PROG = tester.sh
TESTDIR = unittests/
PROGRAM = lyt


OBJS = parser.o converter.o executor.o \
       main.o lexer.o utils.o power.o debugger.o

LDFLAGS = -Lyes -fPIC
CFLAGS = -Werror -Wall
LIBS = -lm
CC=gcc

%.o: %.c
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

all: $(PROGRAM)

clean:
	$(RM) $(OBJS) $(PROGRAM)
	$(MAKE) -C $(TESTDIR) PROGRAM=$(PROGRAM) clean

test: all
	$(INSTALL) $(PROGRAM) $(TESTDIR)
	$(MAKE) -C $(TESTDIR) test PROGRAM=$(PROGRAM)

debug: CFLAGS += -g -DDEBUG
debug: all

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@
