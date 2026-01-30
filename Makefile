PROGRAMS = validator
INCLUDES = -I.

CFLAGS = -g -Wall $(INCLUDES)

all: $(PROGRAMS)

%.o:%.c
	$(CC) $(CFLAGS) -c $<

KEEP_OBJS = utils.o sim.o
TESTER_OBJS = tester.o main.o instruction.o

validator: $(TESTER_OBJS)
	$(CC) -o validator $(CFLAGS) $(TESTER_OBJS) $(KEEP_OBJS)

clean:
	rm -rf $(PROGRAMS) $(TESTER_OBJS) core
