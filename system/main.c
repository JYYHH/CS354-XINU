/*  main.c  - main */

#include <xinu.h>

#define CONTENT "Hello World"
#define LEN 11
#define ADDITION 512 * 3 + 11

process main() {
	// print some constants
	kprintf("LIF_AREA_DIRECT: %d\n", LIF_AREA_DIRECT);
	kprintf("LIF_AREA_INDIR: %d\n", LIF_AREA_INDIR);
	kprintf("LIF_AREA_2INDIR: %d\n", LIF_AREA_2INDIR);
	kprintf("LIF_AREA_3INDIR: %d\n", LIF_AREA_3INDIR);
		// These 4 are some constant for the size

	lifscreate(RAM0, 128, 2000000); // initialize the ramdisk

	did32 fd;
	int status;
	int i;

	// open the file for write

	fd = open(LIFILESYS, "index.txt", "rw");
	kprintf("fd: %d\n", fd);

	// test putc
	for (i = 0; i < 888888; i++){
		status = putc(fd, 'A' + (i % 25));
	}
	kprintf("status for putc: %d\n", status);

	status = close(fd);
	kprintf("status for close: %d\n", status);

	// Open Another file
	fd = open(LIFILESYS, "666.txt", "rw");
	for (i = 0; i < 888888; i++)
		putc(fd, 'Z');
	close(fd);

	// re-open the file for read
	fd = open(LIFILESYS, "index.txt", "r");
	kprintf("fd: %d\n", fd);

	int pos;
	for (i = 0; i < 100000; i++){
		pos = rand() % 888888;
		seek(fd, pos);
		if (getc(fd) != 'A' + (pos % 25)){
			kprintf("Error at position %d, quit!\n", pos);
			exit();
		}
	}
	

	kprintf("******* PASS ALL TEST CASES! ************\n");

}
