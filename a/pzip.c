#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if(argc < 2){
		// usage
		printf("my-zip: file1 [file2 ...]\n");
		return 1;
	}
	
	FILE * fp;

	int first = -1;
	int curr;
	size_t runLength = 0;


	for(size_t i = 1; i < argc; i++){
		fp = fopen(argv[i], "r");
		if (fp==NULL){
			printf("my-zip: cannot open file\n");
			exit(1);
		}
		
		while ( (curr = fgetc(fp)) != EOF ){
			if(first == -1) first = curr;
			
			if( curr == first ) runLength++; 
			else {
				// end of run
				//printf("%ld%c", runLength, (char)first);
				fwrite(&runLength, 4, 1, stdout);
				putc((char)first, stdout);
				first = curr;
				runLength = 1;
			}
		}
		fclose(fp);
	}
	//printf("%ld%c", runLength, (char)first);
	fwrite(&runLength, 4, 1, stdout);
	putc((char)first, stdout);

  	return 0;
}
