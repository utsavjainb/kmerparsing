#include <zlib.h>
#include <iostream>
#include <stdio.h>
#include <numa.h>
#include "kseq.h"
#include "dna.h"
#include "calc.h"
#include "numa.hpp"
#include <string>
#include <vector>
#include <pthread.h>
#include "libfipc_test_time.h"
KSEQ_INIT(gzFile, gzread)

//#define NUM_THREADS 2

Numa n;
std::vector <numa_node> nodes = n.get_node_config();
long pgsize = 4096;

struct thread_data {
	int thread_id;
	long start;
	long end;
	std::string fname;	
};





void gen_kmers_ascii(std::string seq, int k) {
	int n = seq.length();
	
	for(int i = 0; i < n - k + 1; i++){
		std::cout << seq.substr(i, k) << std::endl;
		//void* seq_start = (void*)i;
		//cpu = to_cpu(seq_start);
		//enqueue(cpu, seq_start);		
	}  

}


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

		//std::cout << seq->seq.s << std::endl;
		//gen_kmers_ascii(seq->seq.s, 3);
		//std::cout << seq->seq.l << std::endl;
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

long roundup(long n, long m) {
    return n >= 0 ? ((n + m - 1) / m) * m : (n / m) * m;
}

int spawn_threads(uint32_t num_threads, std::string f){
	std::string f1 = "ntest.fa"; 
	long f_sz = get_file_size(f);	
	std::cout << "f_sz: " << f_sz << std::endl;
	long seg_sz = get_seg_size(f_sz, num_threads);
	std::cout << "seg_sz: " << seg_sz << std::endl;
	
	cpu_set_t cpuset; 

	pthread_t threads[num_threads];
	struct thread_data td[num_threads];
	int rc;	
	long lastend = 0;
	for(unsigned int i = 0; i < num_threads; i++){
		td[i].thread_id = i;
		td[i].fname = f;
		td[i].start = round_down(seg_sz * i, pgsize);
		td[i].end = round_up(seg_sz * (i+1), pgsize);
		if ( i > 0 and td[i].end <= lastend){
			break;
		}	
		lastend = td[i].end;
		std::cout << td[i].start << std::endl;
		std::cout << td[i].end << std::endl;
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

int main(int argc, char *argv[]) {
	if (argc == 1) {
		fprintf(stderr, "Usage: %s <in.seq>\n", argv[0]);
		return 1;
	}
	std::string f(argv[1]);
	uint32_t num_threads = nodes[0].num_cpus;	
	spawn_threads(num_threads, f);	
	return 0;
}
