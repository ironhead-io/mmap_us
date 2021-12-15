//Let's learn how to use mmap()
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BUFSIZE 4096 

int main ( int argc, char *argv[] ) {

	//Define time structures
	struct timespec a, b;	
#if 1
	//Do a write with read/write realloc
	if ( 1 ) {
		clock_gettime( CLOCK_REALTIME, &a );
		int fd = open( "file.big", O_RDONLY ); 
		struct stat sb = {0};
		fstat( fd, &sb );
		fprintf( stderr, "Attempting copying %ldMB file via standard routine\n",
	 		sb.st_size / 1024 / 1024 );

		int newfd = open( "standard.big", O_CREAT | O_WRONLY, 0755 );
		int size = sb.st_size;
		
		while ( size ) {
			unsigned char d[ BUFSIZE + 1 ] = { 0 };
			ssize_t bytes_read = read( fd, d, BUFSIZE ); 
			ssize_t bytes_written	= write( newfd, d, bytes_read );
			size -= bytes_written;
		}

		clock_gettime( CLOCK_REALTIME, &b );
		fprintf( stderr, "Operation completed in: %ld seconds.\n", 
			b.tv_sec - a.tv_sec );
		close( newfd );
		close( fd );
	}
#endif

	//Do a write with mmap 
	if ( 1 ) {
		clock_gettime( CLOCK_REALTIME, &a );
		int fd = open( "file.big", O_RDONLY ); 
		struct stat sb = {0};
		fstat( fd, &sb );
		fprintf( stderr, "Attempting copying %ldMB file via mmap()\n", 
	 		sb.st_size / 1024 / 1024 );

		off_t off = 0;
		void *orig, *addr;
		addr = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, off );
		if ( addr == MAP_FAILED ) {
			fprintf( stderr, "mmap() failed: %s\n", strerror( errno ) );
			return 1;
		}

		orig = addr;

		int newfd = open( "mmapped.big", O_CREAT | O_WRONLY, 0755 );
		if ( newfd == -1 ) {
			fprintf( stderr, "failed to open new file: %s\n", strerror( errno ) );
			return 1;
		}


		//Error isn't mmap, its out of space... :( - Have no idea how to catch this
		//If write won't tell you... you're kind of screwed...
#if 0
		//In this case, we can just assume write succeeds.  
		int bytes_written = write( newfd, addr, sb.st_size );
	 	if ( bytes_written == -1 || bytes_written < sb.st_size ) {
			fprintf( stderr, "write failed: %s\n", strerror( errno ) );
			return 1;
		}
#else
		//A non-blocking socket (or any other file for that matter) may 
		//only write a few bytes at a time... how do I account for this with mmap()?

		if ( fcntl( newfd, F_SETFL, fcntl( newfd, F_GETFL ) | O_NONBLOCK ) == -1 ) {
			fprintf( stderr, "changing to nonblocking failed: %s\n", strerror( errno ) );
			return 1;
		}

		for ( int size = sb.st_size, w = 8192; size > 0 ; ) {
			int b = write( newfd, addr, w );
			if ( b == -1 ) {
				//Anything but EAGAIN stops
				if ( errno != EAGAIN ) { 
					fprintf( stderr, "write failed: %s\n", strerror( errno ) );
					return 1;
				}
				continue;
			}
			size -= b;
			addr += w;
		}
#endif

		if ( munmap( orig, sb.st_size ) == -1 ) {
			fprintf( stderr, "munmap() failed: %s\n", strerror( errno ) );
			return 1;
		}

		clock_gettime( CLOCK_REALTIME, &b );
		fprintf( stderr, "Operation completed in: %ld seconds.", 
			b.tv_sec - a.tv_sec );
		close( newfd );
		close( fd );
	}

	return 0;
}
