long get_file_size(std::string filename) {
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    int size = ftell(p_file);
    fclose(p_file);
    return size;
}

long get_seg_size(long file_sz, uint32_t num_threads){
	return file_sz / num_threads;	

}

long round_down(long n, long m) {
    return n >= 0 ? (n / m) * m : ((n - m + 1) / m) * m;
}


long round_up(long n, long m) {
    return n >= 0 ? ((n + m - 1) / m) * m : (n / m) * m;
}
