/*
///////////////////////////////////////////////////////////////////////////////
////// CCCC ///////////////////////////////////////////////////////////////////
//// C     C //////////////////////////////////////////////////////////////////
/// C        //////////////////////////////////////////////////////////////////
/// C        //////////////////////////////////////////////////////////////////
/// C        //////////////////////////////////////////////////////////////////
//// C     C //////////////////////////////////////////////////////////////////
////// CCCC ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <pthread.h>

struct segment {
	char * ptr;
	int len;	//at most chunkSize
	int segNum;
	struct segment *next;
	int last;	//flag for if this is the last segment to be zipped (globally);
};
struct zipped {
	int * counts;
	char * values;
	int numruns;
};

// Lock and CV's
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t seg = PTHREAD_COND_INITIALIZER; 	//producers
pthread_cond_t zip = PTHREAD_COND_INITIALIZER; 	//consumers

#define chunkSize 0x100000 		//1MB
#define MAXCHUNKS 100
struct segment * first;
struct segment * last;
struct segment * next;
int numChunks;																	// say "nun chucks"
int totalChunks;															
struct zipped * results;

// Consumer	
void *zipSegment(void *arg){
	while(1){
		pthread_mutex_lock(&m);
		printf("Segment %d, numChunks %d: ptr=%p, next=%p, last=%d\n", next->segNum, numChunks,next->ptr, next->next, next->last);
		
		if(next->last){															// NOTHING LEFT TO ZIP--TERMINATE
			pthread_cond_signal(&zip);
			pthread_mutex_unlock(&m);
			return NULL;											
		}

		while(next->next == NULL || numChunks < 0){
			if(next->last){															// NOTHING LEFT TO ZIP--TERMINATE
				pthread_cond_signal(&zip);
				pthread_mutex_unlock(&m);
				return NULL;											
			}
			pthread_cond_wait(&zip, &m);
		}
		struct segment *mySeg = next;
		next = next->next;													// move to next segment
		numChunks--;
		if(numChunks < 0)
			printf("NUMCHUNKS IS NEGATIVE!!\n");
		//printf("Allocate results (array of zipped structs)\n");
		//make sure we have space to store this result
		if (results == NULL || (results+totalChunks) == NULL)
			results = realloc(results,sizeof(struct zipped)*(totalChunks+1));

		//printf("Allocate result\n");
		//make room for worst case zipped result
		struct zipped * result = (struct zipped *)calloc(sizeof(struct zipped), 1);
		//result->counts = (int *)malloc(sizeof(int)*(mySeg->len+1));
		//if(result->counts == NULL) printf("malloc result->counts failed!\n");
		//result->values = (char *)malloc(sizeof(char)*(mySeg->len+1));
		//if(result->values == NULL) printf("malloc result->values failed!\n");

		//do zip
		int firstChar = -1;
		char * curChar;
		int runLength = 0;
		//zip chunk
		result->numruns = 0;
		for(int i = 0; i < mySeg->len ; i++){
			curChar = (mySeg->ptr+i);
			if(firstChar == -1) firstChar = *curChar;
			if(*curChar == firstChar) runLength++;
			else {										// end of run
				//*(result->counts + result->numruns) = runLength;
				//*(result->values + result->numruns) = *curChar;
				firstChar = *curChar;
				runLength = 1;
				result->numruns++;
			}
		}
		if(runLength > 0){
			//*(result->counts + result->numruns) = runLength;
			//*(result->values + result->numruns) = *curChar;
			result->numruns++;
		}

		//printf("Reallocate result\n");
		//reallocate to actual size of zipped results
		//result->counts = (int *)realloc(result->counts, (result->numruns+1)*sizeof(int));
		//if(result->counts == NULL) printf("realloc result->counts failed!\n");
		//result->values = (char *)realloc(result->values, (result->numruns+1)*sizeof(char));
		//if(result->values == NULL) printf("realloc result->values failed!\n");

		results[mySeg->segNum] = *result;
		//free(result);


		//printf("Segment %d finished. numruns=%d, last run=%c-%d, numChunks=%d\n", mySeg->segNum, result->numruns,*(result->values+result->numruns-1),*(result->counts+result->numruns-1), numChunks);
		//free(mySeg);

		pthread_cond_signal(&seg);
		pthread_mutex_unlock(&m);
	}
	printf("Returning\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	// cmd syntax check
	if(argc < 2){
		// usage
		printf("my-zip: file1 [file2 ...]\n");
		return 1;
	}
	// 1 thread per available processor
	int nthreads = get_nprocs();
	pthread_t cid[nthreads];

	int fp;
	char * srcs[argc-1];
	struct stat statbuff[argc-1];
	numChunks = 1;						
	totalChunks = 0;
	int numFiles = argc-1;
	first = (struct segment *)malloc(sizeof(struct segment));
	last = first;
	next = last;

	//MMAP all our files to srcs[]
	for(int i = 0; i < numFiles; i++){
		// open file i
		if ((fp = open(argv[i+1], O_RDONLY)) < 0){
			printf("my-zip: cannot open file\n");
			exit(1);
		}
		// get file sizes
		if (fstat (fp, &statbuff[i]) < 0){
			printf("fstat error\n");
			exit(1);
		}
		//printf("File %d size = %d\n", i+1, (int)statbuff[i].st_size);
		// mmap files
		if ((srcs[i] = mmap (0, statbuff[i].st_size, PROT_READ, MAP_SHARED, fp, 0)) == (caddr_t) -1){
			printf ("mmap error for input");
			return 0;
		}
		// close file i
		close(fp);	
	}

	// Build linked list of segments
	first->segNum = 0;
	first->ptr = NULL;
	first->next = NULL;
	first->last = 0;
	next = first;
	// make consumer threads 
	printf("%d Consumers made\n", nthreads);
	for(int i = 0; i < nthreads ; i++)
		pthread_create(&cid[i], NULL, zipSegment, NULL);

	// Producer
	for(int i = 0; i < numFiles; i++){
		int size = statbuff[i].st_size;
		char * position = srcs[i];

		//while this file can still be segmented
		while(size > 0){
			//Grab Lock 
			pthread_mutex_lock(&m);
			while (numChunks == MAXCHUNKS)
				pthread_cond_wait(&seg, &m);

			numChunks++;
			totalChunks++;
			struct segment *newSeg = (struct segment * )malloc(sizeof(struct segment));
			last->next = newSeg;
			newSeg->segNum = last->segNum+1;
			//base case: size of rest of file is less than chunk size -- go to next file
			if(size < chunkSize){
				last->len = size;
				last->ptr = position;
				size -= chunkSize;
				printf("This segment: len=%d, ptr=%p, size=%d, last->next=%p\n", last->len, last->ptr, size, last->next);
				last = newSeg;
			}
			else{
				last->len = chunkSize;
				last->ptr = position;
				position += chunkSize;
				size -= chunkSize;
				printf("This segment: len=%d, ptr=%p, size=%d, last->next=%p\n", last->len, last->ptr, size, last->next);
				last = newSeg;	
			}
			if(i+1 == numFiles && size <= 0){	// set the last segment's last flag to true
				printf("oooooeeee!!!!!!!!!!!************88\n");
				last->last = 1;
			}
			pthread_cond_signal(&zip);
			pthread_mutex_unlock(&m);
		}
	}
	printf("Producer finished: totalChunks=%d\n", totalChunks);
	//wait for consumers
	for(int i = 0; i < nthreads ; i++)
		pthread_join(cid[i], NULL);

	return 0;
}
