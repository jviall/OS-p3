/*
///////////////////////////////////////////////////////////////////////////////
///////CCCC////////////////////////////////////////////////////////////////////
/////C     C///////////////////////////////////////////////////////////////////
////C//////////////////////////////////////////////////////////////////////////
////C//////////////////////////////////////////////////////////////////////////
////C//////////////////////////////////////////////////////////////////////////
/////C/////C///////////////////////////////////////////////////////////////////
///////CCCC////////////////////////////////////////////////////////////////////
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>

struct segment {
	char * ptr;
	int len;	//at most chunkSize
	int segNum;
	struct segment *next;
};

int main(int argc, char *argv[]) {
	// cmd syntax check
	if(argc < 2){
		// usage
		printf("my-zip: file1 [file2 ...]\n");
		return 1;
	}
	// 1 thread per available processor
	//int nthreads = get_nprocs();

	int fp;
	char * srcs[argc-1];
	struct stat statbuff[argc-1];
	size_t chunkSize = 0x100000; 	//1MB
	int numChunks = 0;						// say "nun chucks"
	struct segment * first = malloc(sizeof(struct segment));

	//MMAP all our files to srcs[]
	for(int i = 1; i < argc; i++){
		// open file i
		if ((fp = open(argv[i], O_RDONLY)) < 0){
			printf("my-zip: cannot open file\n");
			exit(1);
		}
		// get file sizes
		if (fstat (fp, &statbuff[i-1]) < 0){
			printf("fstat error\n");
			exit(1);
		}
		printf("File %d size = %d\n", i, (int)statbuff[i-1].st_size);
		// mmap files
		if ((srcs[i-1] = mmap (0, statbuff[i-1].st_size, PROT_READ, MAP_SHARED, fp, 0)) == (caddr_t) -1){
			printf ("mmap error for input");
			return 0;
		}
		// close file i
		close(fp);	
	}

	// Build linked list of segments
	first->segNum = 1;
	struct segment * curChunk = first;
	for(int i = 1; i < argc; i++){
		int size = statbuff[i-1].st_size;
		char * position = srcs[i-1];
		//first segment
		if(i == 1){
			first->ptr = position;
			numChunks++;
			//base case: size of rest of file is less than chunk size
			if(size < chunkSize){
				first->len = size;
				continue;
			}
			first->len = chunkSize;
			position += chunkSize;
			size -= chunkSize;
		}
		//while this file can still be segmented
		while(size > 0){
			numChunks++;
			struct segment *next = malloc(sizeof(struct segment));
			curChunk->next = next;
			//base case: size of rest of file is less than chunk size -- go to next file
			if(size < chunkSize){
				next->segNum = curChunk->segNum+1;
				next->len = size;
				next->ptr = position;
				curChunk = next;
				break;
			}
			//
			else{
				next->segNum = curChunk->segNum+1;
				next->len = chunkSize;
				next->ptr = position;
				position += chunkSize;
				size -= chunkSize;
				curChunk = next;	
			}
		}
	}//end linked list building
	
	//Zipperino
	int firstChar = -1;
	char * curChar;
	size_t runLength = 0;
	curChunk = first;
	while(curChunk){
		//printf("Total Chunks = %d | numChunk: %d | chunk length: %d\n", 
				//numChunks, curChunk->segNum, curChunk->len);
		//zip chunk
	  for(int i = 0; i < curChunk->len ; i++){
			curChar = (curChunk->ptr+i);
			if(firstChar == -1) firstChar = *curChar;
			if(*curChar == firstChar) runLength++;
			else {	// end of run
				printf("%ld-%c", runLength, (char)(firstChar));
				//fwrite(&runLength, 4, 1, stdout);
				//putc((char)firstChar, stdout);
				firstChar = *curChar;
				runLength = 1;
			}
		}
		struct segment *prev = curChunk;
		curChunk = curChunk->next;
		free(prev); 
	}
	printf("%ld-%c", runLength, (char)(firstChar));
	return 0;
}
