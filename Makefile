CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

c38: $(OBJS)
	$(CC) -o c38 $(OBJS) $(LDFLAGS)

out: $(OBJS)
	$(CC) -g -o out $(OBJS) $(LDFLAGS)

$(OBJS): c38.h

test: c38 
	./test.sh

deb: c38
	./deb.sh

clean:
	rm -f c38 *.o *~ tmp* out

.PHONY: test clean deb

