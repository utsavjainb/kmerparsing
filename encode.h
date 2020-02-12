#include <ctype.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <map>

std::map<char, int> encoding_map;
encoding_map.insert('A', 0b00);
encoding_map.insert('C', 0b01);
encoding_map.insert('T', 0b10);
encoding_map.insert('G', 0b11);


int encode_sequence(std::string seq) {
	
	for(char& c : seq) {
		std::cout << encoding_map[c] << std::endl;
	}
	return 0;

}



int main(int argc, char *argv[])  {
	std::string s1 = "ACTG"; 
	encode_sequence(s1);	
}
