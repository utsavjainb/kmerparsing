#include <ctype.h>
#include<iomanip>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "dna_bitset.h"


std::map<int,int> encoding_map{ {65,0b00} , {67,0b01} , {71,0b10} , {84,0b11}};


void combine(int&a, int b){
	std::cout << std::setw(8) << std::setfill('0') << a << std::endl;		
	a >> 2;
	std::cout << a << std::endl;		
	a |= encoding_map[b];	
	std::cout << a << std::endl;		
//a *= 10; 
	//a += encoding_map[b];

}

int encode_sequence(std::string seq) {
	int bin_rep = 0;
		
	for(char& c : seq) {
		combine(bin_rep, c);
	}
	return bin_rep;

}



int main(int argc, char *argv[])  {
	std::string s1 = "ACTG"; 
	DnaBitset d(s1.c_str(), 4);
	std::cout << d << std::endl;	
	
}
