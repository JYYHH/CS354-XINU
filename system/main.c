/*  main.c  - main */

#include <xinu.h>

#define CONTENT "Hello World"
#define LEN 11
#define ADDITION 512 * 3 + 11
#define NUM_WRITTEN 888888

process main() {
	// print some constants
	kprintf("LIF_AREA_DIRECT: %d\n", LIF_AREA_DIRECT);
	kprintf("LIF_AREA_INDIR: %d\n", LIF_AREA_INDIR);
	kprintf("LIF_AREA_2INDIR: %d\n", LIF_AREA_2INDIR);
	kprintf("LIF_AREA_3INDIR: %d\n", LIF_AREA_3INDIR);
		// These 4 are some constant for the size

	lifscreate(RAM0, 128, 2000000); // initialize the ramdisk

	did32 fd1, fd2;
	int status;
	int i, len1, len2;
	len1 = len2 = 0;

	// open the file for write

	fd1 = open(LIFILESYS, "index.txt", "rw");
	fd2 = open(LIFILESYS, "content.txt", "rw");
	kprintf("fds: %d %d\n", fd1, fd2);

	// test putc
	for (i = 0; i < NUM_WRITTEN * 2; i++){
		if ((rand() & 1) && (len1 < NUM_WRITTEN)){
			status = putc(fd1, 'A' + (len1 % 25));
			len1 ++;
		}
		else{
			status = putc(fd2, 'a' + (len2 % 25));
			len2 ++;
		}
		
	}

	status = close(fd1);
	status = close(fd2);

	// re-open the file for read
	fd1 = open(LIFILESYS, "index.txt", "r");
	fd2 = open(LIFILESYS, "content.txt", "r");
	kprintf("fds: %d %d\n", fd1, fd2);

	int pos;
	char get_;
	for (i = 0; i < 200000; i++){
		pos = rand() % NUM_WRITTEN;
		if (rand() & 1){
			seek(fd1, pos);
			get_ = getc(fd1);
			if (get_ != 'A' + (pos % 25)){
				kprintf("Error fd1 at position %d, quit!\n", pos);
				kprintf("Should be %c, but get %c\n", 'A' + (pos % 25), get_);
				exit();
			}
		}
		else{
			seek(fd2, pos);
			get_ = getc(fd2);
			if (get_ != 'a' + (pos % 25)){
				kprintf("Error fd2 at position %d, quit!\n", pos);
				kprintf("Should be %c, but get %c\n", 'a' + (pos % 25), get_);
				exit();
			}
		}
	}
	

	kprintf("******* PASS ALL TEST CASES! ************\n");

}
