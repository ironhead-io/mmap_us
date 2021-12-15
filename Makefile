# By default, run this test with files that are 1G large
SIZE=1000000000

build: file.big
	$(CC) -Wall -Werror -o mmap mmap.c

file.big:
	head -c $(SIZE) /dev/urandom > file.big

#.PHONY: file.big
