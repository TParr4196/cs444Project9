CC=gcc
CCOPTS=-Wall -Wextra -Werror -DCTEST_ENABLE
LIBS=libvvsfs.a

SRCS=$(wildcard *.o)
TARGETS=$(SRCS:.c=)

.PHONY: clean pristine test

testfs: libvvsfs.a
	$(CC) $(CCOPTS) -o $@ $(LIBS)

test: testfs
	./testfs

libvvsfs.a: block.o image.o free.o testfs.o pack.o dir.o inode.o
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CCOPTS) -c $^

clean:
	rm -f $(TARGETS)
pristine: clean
	rm -f libvvsfs.a testfs image.txt