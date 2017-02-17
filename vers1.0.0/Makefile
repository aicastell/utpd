CC=gcc
CFLAGS=-W -Wall -Winline -ggdb3
TARGET=utpd
OBJS=error.o utpd.o queue.o queue_item.o
LIBS=-lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)

test: clean test_queue

test_queue:
	$(CC) -W -Wall -ggdb3 -c queue.c
	$(CC) -W -Wall -ggdb3 -c queue_item.c
	$(CC) -W -Wall -ggdb3 -c test_queue.c
	$(CC) -W -Wall -ggdb3 -c error.c
	$(CC) queue.o queue_item.o test_queue.o error.o -o test_queue

install:
	sudo cp utpd /usr/bin
	sudo chmod 755 /usr/bin/$(TARGET)
	sudo strip /usr/bin/$(TARGET)
	sudo cp -Rfa etc /

clean:
	rm -f *.o $(TARGET) test_queue queue_item

