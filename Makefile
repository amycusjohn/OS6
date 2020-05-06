CC = gcc
CFLAGS = -g

TARGET1 = oss
TARGET2 = child
OBJS1 = oss.o
OBJS2 = child.o
HEADER = helper.h

all: $(TARGET1) $(TARGET2)

oss: $(OBJS1)
	$(CC) $(CFLAGS)  -g -Wall -lpthread -lrt -lm -o oss oss.o

user: $(OBJS2)
	$(CC) $(CFLAGS)  -g -Wall -lpthread -lrt -lm -o child child.o

%.o: %.c  $(HEADER)
	gcc -c $(CFLAGS) $*.c -o $*.o

clean:
	/bin/rm -f *.o *.txt oss child
