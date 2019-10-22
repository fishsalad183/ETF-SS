#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "instruction.h"
#include "symbol.h"
#include "relocation.h"


using namespace std;



class Assembler {
public:
	
	void assemble(ifstream& ifs, ofstream& ofs, int startAddress);

private:

	int locationCounter = 0;

	void firstPass(ifstream& ifs, int startAddress);
	void secondPass(ifstream& ifs);

	TokenType parseToken(string token);
	Operands numberOfOperands(string instructionToken);

	vector<Symbol> symbolTable;
	void addSymbol(string name, Section* section, int offset, bool isGlobal);
	Symbol* findByName(string& name);

	vector<Relocation> relocations;

	vector<string> errorList;
	void error(string description, bool fatal);

	int processInstruction(Entry* entry, Section* section, string instructionToken, string firstOperand = "", string secondOperand = "");
	int operandToMask(Entry* entry, Section* section, string operand, int*& additionalBytes);
	Symbol* processSymbol(Entry* entry, Section* section, string symbol, RelType relType);
	bool isImmediate(string operand);
	int evaluateExpression(string expression);

	void print(ostream& ofs);
	void printRelocationTable(ostream& ofs, const Section* s);
	
};