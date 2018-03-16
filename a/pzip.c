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

	//int first = -1;
	//int curr;
	//size_t runLength = 0;


	for(size_t i = 1; i < argc; i++){
		if ((fp = open(argv[i], O_RDONLY)) < 0){
			printf("my-zip: cannot open file\n");
			exit(1);
		}	
		char * src;
		struct stat statbuff;

		if (fstat (fp, &statbuff) < 0){
			printf("fstat error\n");
			exit(1);
		}
		if ((src = mmap (0, statbuff.st_size, PROT_READ, MAP_SHARED, fp, 0)) == (caddr_t) -1){
			printf ("mmap error for input");
      return 0;
   	}
		///*nice kitty
		for(int i = 0; i < statbuff.st_size; i++)
			printf("%c", *(src + i));
		//*/
	}
	//printf("%ld%c", runLength, (char)first);
	//fwrite(&runLength, 4, 1, stdout);
	//putc((char)first, stdout);

  return 0;
}
