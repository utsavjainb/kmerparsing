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


Numa n;
std::vector <numa_node> nodes = n.get_node_config();
long pgsize = 4096;

struct thread_data {
	int thread_id;
        //start and end locations in FASTA file that show segment thread is resposible for parsing.
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

        //initializes the seq struct in which the sequence data will be stored in 
	seq = kseq_init(fp);

        //each time kseq_read is called, it tries to read the next record starting with >
        //if kseq_read is called at a position in the middle of a sequence, it will skip to the next record
	while ((l = kseq_read(seq)) >= 0) {
		//printf("name: %s\n", seq->name.s);
		//if (seq->comment.l) printf("comment: %s\n", seq->comment.s);
		//printf("seq: %s\n", seq->seq.s);
		//if (seq->qual.l) printf("qual: %s\n", seq->qual.s);

		//std::cout << seq->seq.s << std::endl;
		//gen_kmers_ascii(seq->seq.s, 3);
		//std::cout << seq->seq.l << std::endl;
    	//DnaBitset encoded_seq(seq->seq.s, seq->seq.l);
    	//const char* dna_str_recovered = encoded_seq.to_string();
    	//std::cout << "recovered sequence: " << dna_str_recovered << "\n";
	
		//checking if reached end of assigned segment	
		curr_pos = gztell(fp);
		if(curr_pos >= t_data->end){
			break;
		}	
		
	}
	kseq_destroy(seq);
	gzclose(fp);
	pthread_exit(NULL);
}

int spawn_threads(uint32_t num_threads, std::string f){
        //get size of FASTA file to be read
	long f_sz = get_file_size(f);	
        //divide file size by number of threads
	long seg_sz = get_seg_size(f_sz, num_threads);
	

        //Since the kseq library reads in chunks of 4096 bytes (one page), each thread must be responsible to parse atleast 4096 bytes
	if (seg_sz < 4096) {
		seg_sz = 4096;
	}
	
	cpu_set_t cpuset; 
	pthread_t threads[num_threads];
	struct thread_data td[num_threads];

	int rc;	
	int threads_spawned = num_threads;
	for(unsigned int i = 0; i < num_threads; i++){
                /*
                The starting and ending locations for the memory segment that the thread is responsible for must be multiples of 4096                
                round_up will round number up to nearest multiple of 4096
                    Ex: Filesize = 10000, two threads to parse.   
                    Before rounding:
                        t1.start = 0, t1.end = 5000, t2.start = 5000, t2.end = 10000 
                    After rounding:
                        t1.start = 0, t1.end = 8192, t2.start = 8192, t2.end = 12228 
                */
                
		td[i].start = round_up(seg_sz * i, pgsize);
		td[i].end = round_up(seg_sz * (i+1), pgsize);

                
                // Breaks thread spawn loop if starting location of thread is past file size, saves how many threads are spawned into threads_spawned
		if (td[i].start >= f_sz){
			threads_spawned = i;
			break;
		}

		td[i].thread_id = i;
		td[i].fname = f;
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
        
        //joins the threads that were spawned
   	for(unsigned int i = 0; i < threads_spawned; i++){
		pthread_join(threads[i], NULL);
	}	

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
