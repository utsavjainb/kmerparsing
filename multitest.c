#include <zlib.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include "kseq.h"
//#include "bgzf.h"
#include "libfipc_test_time.h"
KSEQ_INIT(gzFile, gzread)
//KSEQ_INIT(BGZF*, gzread)

int main(int argc, char *argv[])
{
	gzFile fp;
	//BGZF* fp;
	kseq_t *seq;
	int l;
	if (argc == 1) {
		fprintf(stderr, "Usage: %s <in.seq>\n", argv[0]);
		return 1;
	}
	int j;
	u_int64_t totcycles; 
	for (j = 0; j < 1;j++) {
			
			fp = gzopen(argv[1], "r");
			//fp = bgzf_open(argv[1], "r");
			
			//gzseek(fp, 0L, SEEK_END);
			//int sz = gztell(fp);
			//printf("fsize: %d\n", sz);	
			//gzseek(fp, 0L, SEEK_SET);

	 		//int currpos;	
			//int pos = gzseek(fp, 1000, SEEK_SET);	
			int currpos = gztell(fp);
			printf("currpos: %d\n", currpos);	
				
			seq = kseq_init(fp);
			currpos = gztell(fp);
			printf("currpos: %d\n", currpos);	
			u_int64_t start, end;

			start = RDTSC_START();

			while ((l = kseq_read(seq)) >= 0) {
				currpos = gztell(fp);
				printf("currpos: %d\n", currpos);	
				/*
				printf("name: %s\n", seq->name.s);
				if (seq->comment.l) printf("comment: %s\n", seq->comment.s);
				printf("seq: %s\n", seq->seq.s);
				if (seq->qual.l) printf("qual: %s\n", seq->qual.s);
				*/
				//int currpos = gztell(fp);
				//currpos = gzseek(fp, 0L, SEEK_SET);
				//printf("currpos: %d\n", currpos);	
				//printf("sz: %lu\n", sizeof(seq));	
	  			//int a = system("echo 3 > /proc/sys/vm/drop_caches");
				
			}
			end = RDTSCP();
			u_int64_t cycles = (end - start);
			totcycles += cycles;
			kseq_destroy(seq);
			gzclose(fp);
	  		//int a = system("echo 3 > /proc/sys/vm/drop_caches");
	}
	//printf("%lu\n", totcycles);
	return 0;
}
