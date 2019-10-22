#include <iostream>
#include <fstream>

#include "assembler.h"


using namespace std;


int main(int argc, char *argv[]) {

	if (argc < 4) {
		cout << endl << "Insufficient number of command line parameters." << endl;
		return 2;
	}

	char* inputFileName = argv[1];
	ifstream ifs(inputFileName);
	if (!ifs || !ifs.is_open()) {
		cout << endl << "Error opening input file: " << inputFileName << endl;
		return 2;
	}

	char* outputFileName = argv[2];
	ofstream ofs(outputFileName);
	if (!ofs || !ofs.is_open()) {
		cout << endl << "Error opening output file: " << outputFileName << endl;
		return 2;
	}

	int startAddress = atoi(argv[3]);


	Assembler a;
	a.assemble(ifs, ofs, startAddress);
}