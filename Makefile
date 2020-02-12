CC=g++
CCFLAGS=-g -std=c++14 -pthread -O2 -lnuma -lz


#all: clean
#	$(CC) $(CCFLAGS) main.c -o main 

main:kseq.h main.c
		g++ -std=c++14 -pthread -g -O2 main.c -o main -lz -lnuma

clean:
		rm -f *.o main 
