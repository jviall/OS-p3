#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	if(argc < 2){
		// usage
		printf("my-zip: file1 [file2 ...]\n");
		return 1;
	}

	int fp;

	//char * dest;
	char * srcs[argc-1];
	struct stat statbuff[argc-1];
	//int curr;
	//size_t runLength = 0;

	//MMAP all our files to srcs[]
	for(size_t i = 1; i < argc; i++){
		// open files
		if ((fp = open(argv[i], O_RDONLY)) < 0){
			printf("my-zip: cannot open file\n");
			exit(1);
		}
		// get file sizes
		if (fstat (fp, &statbuff[i-1]) < 0){
			printf("fstat error\n");
			exit(1);
		}
		// mmap files
		if ((srcs[i-1] = mmap (0, statbuff[i-1].st_size, PROT_READ, MAP_SHARED, fp, 0)) == (caddr_t) -1){
			printf ("mmap error for input");
			return 0;
		}	
	}

	///*nice kitty
	for(size_t i = 1; i < argc; i++){
		for(int a = 0; a < statbuff[i-1].st_size; a++)
			printf("%c", *(srcs[i-1] + a));
	}
	//*/

	//printf("%ld%c", runLength, (char)first);
	//fwrite(&runLength, 4, 1, stdout);
	//putc((char)first, stdout);

	return 0;
}
