CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

c38: $(OBJS)
	$(CC) -o c38 $(OBJS) $(LDFLAGS)

$(OBJS): c38.h

test: c38 
	./test.sh

clean:
	rm -f c38 *.o *~ tmp*

.PHONY: test clean

