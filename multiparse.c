#include <zlib.h>
#include <iostream>
#include <stdio.h>
#include "kseq.h"
#include <string>
#include <vector>
#include <pthread.h>
#include "libfipc_test_time.h"
KSEQ_INIT(gzFile, gzread)

#define NUM_THREADS 2


struct thread_data {
	int thread_id;
	std::string fname;	
};


void *readFile(void *threadarg){
	struct thread_data *t_data;
	t_data = (struct thread_data *) threadarg;
	std::cout << "Thread ID: " << t_data->thread_id << std::endl;

	gzFile fp;
	kseq_t *seq;
	int l;
	fp = gzopen((t_data->fname).c_str(), "r");
	seq = kseq_init(fp);

	while ((l = kseq_read(seq)) >= 0) {
		printf("name: %s\n", seq->name.s);
		if (seq->comment.l) printf("comment: %s\n", seq->comment.s);
		printf("seq: %s\n", seq->seq.s);
		if (seq->qual.l) printf("qual: %s\n", seq->qual.s);
		
	}

	kseq_destroy(seq);
	gzclose(fp);
}

void spawnThreads(){
	std::string f1 = "example.fasta"; 
	std::string f2 = "example.fasta"; 
	//std::string f2 = "protein.fa"; 
	std::vector<std::string> files;
	files.push_back(f1);
	files.push_back(f2);



	pthread_t threads[NUM_THREADS];
	struct thread_data td[NUM_THREADS];
	int rc;	
	for(unsigned int i = 0; i < files.size(); i++){
		td[i].thread_id = i;
		td[i].fname = files[i];
		rc = pthread_create(&threads[i], NULL, readFile, (void *)&td[i]);	
	
		if (rc) {
			std::cout << "ERROR" <<  rc << std::endl;
			exit(-1);
		}
	
	}
	pthread_exit(NULL);



}


int main(int argc, char *argv[])
{
	spawnThreads();	
	return 0;
	
}
