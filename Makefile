PROGRAM=MyEth
OBJS=main.o ehter.o arp.o
SRCS=$(OBJS:%.o=%.c)
CFLAGs=-Wall -g
LDFLAGS=-lpthread
$(PROGRAM):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LDLIBS)
