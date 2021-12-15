# mmap() vs userspace buffers

Herein is some C code for testing the speed of userspace buffers vs mmap() for large files.   

The Makefile will generate a 1G file, and the compiled program will use both mmap() and read() -> buffer -> write() to make a copy of said file.   Both methods use the same buffer size, but depending on system settings--this could be one of the reasons why performance is better in one case or another.
