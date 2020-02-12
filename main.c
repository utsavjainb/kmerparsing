#include <zlib.h>
#include <iostream>
#include <stdio.h>
#include <numa.h>
#include "kseq.h"
#include "dna.h"
#include "numa.hpp"
#include <string>
#include <vector>
#include <pthread.h>
#include "libfipc_test_time.h"
KSEQ_INIT(gzFile, gzread)

//#define NUM_THREADS 2

Numa n;
std::vector <numa_node> nodes = n.get_node_config();

struct thread_data {
	int thread_id;
	int start;
	int end;
	std::string fname;	
};


int get_file_size(std::string filename) // path to file
{
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    int size = ftell(p_file);
    fclose(p_file);
    return size;
}



void generate_kmers(k, 

void *parse_thread(void *threadarg){
	struct thread_data *t_data;
	t_data = (struct thread_data *) threadarg;

	gzFile fp;
	kseq_t *seq;
	int l;
	fp = gzopen((t_data->fname).c_str(), "r");
	
	//jump to start of segment
	int pos = gzseek(fp, t_data->start, SEEK_SET);
	int curr_pos; 

	seq = kseq_init(fp);

	while ((l = kseq_read(seq)) >= 0) {
		/*
		printf("name: %s\n", seq->name.s);
		if (seq->comment.l) printf("comment: %s\n", seq->comment.s);
		printf("seq: %s\n", seq->seq.s);
		if (seq->qual.l) printf("qual: %s\n", seq->qual.s);
		*/

		std::cout << seq->seq.s << std::endl;
		std::cout << seq->seq.l << std::endl;
    	DnaBitset encoded_seq(seq->seq.s, seq->seq.l);
    	const char* dna_str_recovered = encoded_seq.to_string();
    	std::cout << "recovered sequence: " << dna_str_recovered << "\n";
	
		//checking if reached end of assigned segment	
		curr_pos = gztell(fp);
		if(curr_pos >= t_data->end){
			break;
		}	
		
	}

	kseq_destroy(seq);
	gzclose(fp);
}



int spawn_threads(uint32_t num_threads){
	std::string f1 = "test.fa"; 
	int f_sz = get_file_size(f1);	
	int seg_sz = f_sz / num_threads;	
	
	cpu_set_t cpuset; 

	pthread_t threads[num_threads];
	struct thread_data td[num_threads];
	int rc;	
	for(unsigned int i = 0; i < 1; i++){
		td[i].thread_id = i;
		td[i].fname = f1;
		td[i].start = seg_sz * i;
		td[i].end = seg_sz * (i+1);
		rc = pthread_create(&threads[i], NULL, parse_thread, (void *)&td[i]);	
	
		if (rc) {
			std::cout << "ERROR" <<  rc << std::endl;
			exit(-1);
		}
		
		CPU_ZERO(&cpuset);

#ifndef NDEBUG
	printf("[INFO] thread: %lu, affinity: %u,\n", i, nodes[0].cpu_list[i]);
#endif
		CPU_SET(nodes[0].cpu_list[i], &cpuset);
		pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
	
	}
	pthread_exit(NULL);

	return 0;

}


int main(int argc, char *argv[])
{
	uint32_t num_threads = nodes[0].num_cpus;	
	std::cout << num_threads << std::endl;
	spawn_threads(num_threads);	
	//std::string f1 = "protein.fa"; 
	//std::cout << "totsz: " << get_file_size(f1) << std::endl;
	return 0;
}
