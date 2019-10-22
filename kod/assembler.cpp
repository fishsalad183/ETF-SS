#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "assembler.h"
#include "instruction.h"


using namespace std;



void Assembler::addSymbol(string name, Section* section, int offset, bool isGlobal) {
	Symbol* s = findByName(name);
	if (s != nullptr) {
		error("Symbol " + name + " already exists in symbol table", true);
		return;
	}

	string sectionName;
	if (section) {
		sectionName = section->name;
	}
	else {
		sectionName = "?";
	}
	Symbol symbol((int) symbolTable.size(), name, sectionName, offset, isGlobal);
	symbolTable.push_back(symbol);
}


Symbol* Assembler::findByName(string& name) {
	for (Symbol& symbol : symbolTable) {
		if (symbol.name == name) {
			return &symbol;
		}
	}
	return nullptr;
}


void Assembler::error(string description, bool fatal) {
	errorList.push_back(description);
	if (fatal) {
		for (string& s : errorList) {
			cout << s << endl;
		}

		int x;
		cin >> x;

		exit(1);
	}
}


TokenType Assembler::parseToken(string token) {
	TokenType ret = ILLEGAL;
	if (token == "psw") {
		return PSW;
	}

	for (auto& pair: Instruction::tokenRegexMap) {
		if (pair.first == SYMBOL || pair.first == PSW) {
			continue;
		}
		if (regex_match(token, pair.second)) {
			ret = (TokenType) pair.first;
		}
	}

	if (ret == ILLEGAL && regex_match(token, Instruction::tokenRegexMap.at(SYMBOL))) {
		return SYMBOL;
	}

	return ret;
}


Operands Assembler::numberOfOperands(string instructionToken) {
	for (auto& pair: Instruction::operandNumRegexMap) {
		if (regex_match(instructionToken, pair.second)) {
			return (Operands) pair.first;
		}
	}
	return ERROR;
}


void Assembler::assemble(ifstream& ifs, ofstream& ofs, int startAddress) {

	firstPass(ifs, startAddress);
	
	secondPass(ifs);

	print(ofs);
	
}


void Assembler::firstPass(ifstream& ifs, int startAddress) {
	
	string line;
	Section* section = nullptr;
	int locationCounter = 0;


	while (getline(ifs, line)) {

		if (ifs.bad()) {
			error("Input stream error", true);
		}

		istringstream iss(line);
		string token;
		bool foundCommandInLine = false;	// U jednoj liniji najvise jedna komanda.

		while (iss >> token) {

			TokenType tokenType = parseToken(token);

			if (tokenType == LABEL) {
				string name = token.substr(0, token.size() - 1);
				if (!section) {	// NE SME LABELA PRE SEKCIJE ?!
					error("Label \"" + name + "\" is before any section", true);
				}
				addSymbol(name, section, locationCounter, false);
			}
			else if (tokenType == SECTION) {
				if (section) {
					startAddress += locationCounter;
				}

				if (token == ".text") {
					section = Section::TEXT;
				}
				else if (token == ".data") {
					section = Section::DATA;
				}
				else if (token == ".rodata") {
					section = Section::RODATA;
				}
				else if (token == ".bss") {
					section = Section::BSS;
				}

				if (section->checkIfFirstAppearance() == false) {	// SEKCIJA SME DA SE POJAVLJUJE SAMO JEDANPUT
					error("Section " + section->name + " appeared more than once", true);
				}
				
				section->startAddress = startAddress;

				locationCounter = 0;

				addSymbol(token, section, 0, false);
			}
			else if (tokenType == INSTRUCTION) {
				if (section != Section::TEXT) {
					error("Instruction(s) outside .text section: " + token, true);
				}

				int size = 2;
				Operands op = numberOfOperands(token);

				if (op == NO_OPERANDS) {
					string newToken;
					iss >> newToken;
					if (newToken != "") {
						error("Operand number/syntax error: " + token + " " + newToken, false);
					}
				}
				else if (op == ONE_OPERAND) {
					string operand;
					iss >> operand;
					TokenType operandType = parseToken(operand);
					if (Instruction::isOperand(operandType)) {
						if (Instruction::requiresFourBytes(operandType)) {
							size = 4;
						}
						string newToken;
						iss >> newToken;
						if (newToken != "") {
							error("Operand number/syntax error: " + token + " " + operand + " " + newToken, false);
						}
					}
					else {
						error("Operand syntax error: " + token + " " + operand, true);
					}
				}
				else if (op == TWO_OPERANDS) {
					bool fourBytesRequired = false;
					string operand;
					iss >> operand;
					if (operand.back() == ',') {
						operand.pop_back();
					}
					else {
						error("No comma after operand: " + token + " " + operand, false);
					}
					TokenType operandType = parseToken(operand);
					if (Instruction::isOperand(operandType)) {
						if (Instruction::requiresFourBytes(operandType)) {
							size = 4;
							fourBytesRequired = true;
						}
						string secondOperand;
						iss >> secondOperand;
						operandType = parseToken(secondOperand);
						if (Instruction::isOperand(operandType)) {
							if (Instruction::requiresFourBytes(operandType)) {
								if (fourBytesRequired) {
									error("Two operands requiring two additional bytes in one instruction: " + token + " " + operand + ", " + secondOperand, true);
								}
								else {
									size = 4;
								}
							}
							string newToken;
							iss >> newToken;
							if (newToken != "") {
								error("Operand number/syntax error: " + token + " " + operand + ", " + secondOperand + " " + newToken, false);
							}
						}
						else {
							error("(Second) operand syntax error for " + token + ": " + secondOperand, true);
						}
					}
					else {
						error("(First) operand syntax error for " + token + ": " + operand, true);
					}
				}
				else {
					error("Instruction error: " + token, false);	// sta?
				}

				locationCounter += size;
			}
			else if (tokenType == DIRECTIVE) {
				if (token == ".char" || token == ".word" || token == ".long") {
					string val;
					iss >> val;
					int rep = 1;
					while (val.back() == ',') {
						val.pop_back();
						TokenType type = parseToken(val);
						if (!(type == IMM || type == IMM_HEX || type == EXPRESSION)) {
							error("Directive syntax error", true);
						}
						++rep;
						iss >> val;
					}

					TokenType type = parseToken(val);
					if (!(type == IMM || type == IMM_HEX || type == EXPRESSION)) {
						error("Directive syntax error", true);
					}

					if (token == ".char") {
						locationCounter += 1 * rep;
					}
					else if (token == ".word") {
						locationCounter += 2 * rep;
					}
					else /*if (token == ".long")*/ {
						locationCounter += 4 * rep;
					}
				}
				else if (token == ".align" || token == ".skip") {
					string val;
					iss >> val;
					if (val.back() == ',') {
						val.pop_back();
						string padding;
						iss >> padding;
						TokenType type = parseToken(padding);
						if (!(type == IMM || type == IMM_HEX)) {
							error("Directive syntax error", true);
						}
					}
					TokenType type = parseToken(val);
					if (!(type == IMM || type == IMM_HEX)) {
						error("Directive syntax error", true);
					}

					if (token == ".skip") {
						int bytes = stoi(val, nullptr, 0);
						locationCounter += bytes;
					}
					else /*if (token == ".align")*/ {
						int power = stoi(val, nullptr, 0);
						// SPRECAVANJE GRESAKA?
						int alignment = 1;
						for (int i = 0; i < power; i++) {
							alignment *= 2;
						}
						int over = locationCounter % alignment;
						if ((alignment != 1) && (over != 0)) {
							locationCounter += (alignment - over);
						}
					}
					
				}
			}
			else if (tokenType == END) {
				return;
			}


			if (tokenType == SECTION || tokenType == DIRECTIVE || tokenType == INSTRUCTION) {
				if (!foundCommandInLine) {
					foundCommandInLine = true;
				}
				else {
					error("Only one command allowed per line of code: \"" + token + "\" is breaking the rule", false);
					break;
				}
			}
			
		}
	}

}


void Assembler::secondPass(ifstream& ifs) {

	ifs.clear();
	ifs.seekg(0, ios::beg);


	string line;
	Section* section = nullptr;
	int locationCounter = 0;

	while (getline(ifs, line)) {

		if (ifs.bad()) {
			error("Input stream error", true);
		}

		istringstream iss(line);
		string token;

		while (iss >> token) {

			TokenType tokenType = parseToken(token);

			if (tokenType == GLOBAL) {
				string t;
				while (iss >> t) {
					if (t.back() == ',') {
						t.pop_back();
					}
					Symbol* s;
					s = findByName(t);
					if (s != nullptr) {
						s->isGlobal = true;
					}
					else {
						error(".global directive for unknown symbol", false);
					}
				}
			}
			else if (tokenType == SECTION) {
				if (token == ".text") {
					section = Section::TEXT;
				}
				else if (token == ".data") {
					section = Section::DATA;
				}
				else if (token == ".rodata") {
					section = Section::RODATA;
				}
				else if (token == ".bss") {
					section = Section::BSS;
				}

				locationCounter = 0;

			}
			else if (tokenType == INSTRUCTION) {
				Entry entry;
				entry.offset = locationCounter;

				Operands op = numberOfOperands(token);

				if (op == NO_OPERANDS) {
					processInstruction(&entry, section, token);
				}
				else if (op == ONE_OPERAND) {
					string operand;
					iss >> operand;
					processInstruction(&entry, section, token, operand);
				}
				else {
					string firstOperand, secondOperand;
					iss >> firstOperand;
					if (firstOperand.back() == ',') {
						firstOperand.pop_back();
					}
					iss >> secondOperand;
					processInstruction(&entry, section, token, firstOperand, secondOperand);
				}

				if (entry.size > 0) {
					locationCounter += entry.size;
				}
				else {	// entry.size JE -1 U SLUCAJU DA SU DODATNA 2 BAJTA NEPOZNATA
					locationCounter += 4;
				}

				section->addEntry(entry);

			}
			else if (tokenType == DIRECTIVE) {
				if (token == ".skip" || token == ".align") {
					Entry entry;
					entry.offset = locationCounter;
					entry.value = 0;

					string num;
					iss >> num;
					if (num.back() == ',') {
						num.pop_back();
						string padding;
						iss >> padding;
						int value = stoi(padding, nullptr, 0);
						value &= 0xFF;
						for (int shl = 8; shl <= 24; shl += 8) {
							value |= (value << shl);
						}
						entry.value = value;
					}

					if (token == ".skip") {
						int bytes = stoi(num, nullptr, 0);
						if (bytes > 0) {
							entry.size = bytes;
							section->addEntry(entry);
							locationCounter += bytes;
						}
						else {
							error("Bad number of bytes for .skip", false);
						}
					}
					else /*if (token == ".align")*/ {
						int power = stoi(num, nullptr, 0);
						int alignment = 1;
						for (int i = 0; i < power; i++) {
							alignment *= 2;
						}
						int over = locationCounter % alignment;
						if ((alignment != 1) && (over != 0)) {
							entry.size = alignment - over;
						}
						else {
							entry.size = 0;
						}

						if (entry.size != 0) {
							section->addEntry(entry);
							locationCounter += entry.size;
						}
					}
				}
				else if (token == ".char" || token == ".word" || token == ".long") {	
					int size;
					if (token == ".char") {
						size = 1;
					}
					else if (token == ".word") {
						size = 2;
					}
					else /*if (token == ".long")*/ {
						size = 4;
					}

					string val;
					iss >> val;
					while (val.back() == ',') {
						val.pop_back();
						Entry entry;
						entry.offset = locationCounter;
						TokenType type = parseToken(val);
						if (type == EXPRESSION) {
							entry.value = evaluateExpression(val);
						}
						else {
							entry.value = stoi(val, nullptr, 0);
						}
						entry.size = size;
						section->addEntry(entry);
						locationCounter += size;
						iss >> val;
					}
					Entry entry;
					entry.offset = locationCounter;
					TokenType type = parseToken(val);
					if (type == EXPRESSION) {
						entry.value = evaluateExpression(val);
					}
					else {
						entry.value = stoi(val, nullptr, 0);
					}
					entry.size = size;
					section->addEntry(entry);
					locationCounter += size;
				}
			}
			else if (tokenType == END) {
				return;
			}

		}
	}
}


int Assembler::processInstruction(Entry* entry, Section* section, string instructionToken, string firstOperand, string secondOperand) {	// firstOperand i secondOperand imaju default vrednost ""
	ConditionCode cond = Instruction::getCondition(instructionToken);
	const string constInstTok = instructionToken;
	
	if (Instruction::isRet(instructionToken)) {	// ret je pseudoinstrukcija
		instructionToken = "pop";
		switch (cond) {
		case EQ: {
			instructionToken += "eq";
			break;
		}
		case NE: {
			instructionToken += "ne";
			break;
		}
		case GT: {
			instructionToken += "gt";
			break;
		}
		/*
		case AL: {
			instructionToken += "al";
			break;
		}
		*/
		}
		firstOperand = "r7[0]";
	}
	else if (Instruction::isJmp(instructionToken)) {
		TokenType operandType = parseToken(firstOperand);
		if (operandType == SYMBOL) {
			instructionToken = "add";
			switch (cond) {
			case EQ: {
				instructionToken += "eq";
				break;
			}
			case NE: {
				instructionToken += "ne";
				break;
			}
			case GT: {
				instructionToken += "gt";
				break;
			}
			/*
			case AL: {
			instructionToken += "al";
			break;
			}
			*/
			}
			Symbol* s = findByName(firstOperand);
			int nextInstructionOffset = entry->offset + 4;
			int displacement = s->offset - nextInstructionOffset;
			
			secondOperand = to_string(displacement);
			firstOperand = "r7";
		}
		else /*if (operandType == IMM || operandType == IMM_HEX || operandType == VALUE || operandType == LOC || operandType == PSW
			|| operandType == REGDIR || operandType == REGIND_DISP_IMM || operandType == REGIND_DISP_VAR)*/ {
			instructionToken = "mov";
			secondOperand = firstOperand;
			firstOperand = "r7";
		}
	}
	else if (!Instruction::isCall(instructionToken)) {
		TokenType operandType = parseToken(firstOperand);
		if (operandType == SYMBOL) {
			firstOperand = "#" + firstOperand;	// DA BI BIO MEMDIR
		}
		operandType = parseToken(secondOperand);
		if (operandType == SYMBOL) {
			secondOperand = "#" + secondOperand;	// DA BI BIO MEMDIR
		}
	}
	InstructionCode inst = Instruction::getInstruction(instructionToken);

	int code = 0;
	code |= cond;
	code <<= 4;
	code |= inst;
	code <<= 5;


	int* additionalBytes = new int[1];
	*additionalBytes = 0;
	int sizeBasedOnFirstOperand = 0;


	if (firstOperand.empty()) {	// BEZ OPERANADA
		code <<= 5;

		// samo IRET
		// RET se prevodi u pop jer je pseudoinstrukcija
	}
	else if (secondOperand.empty()) {	// IMA JEDAN OPERAND
		int mask = operandToMask(entry, section, firstOperand, additionalBytes);

		switch (inst) {
		case PUSH: {
			code <<= 5;
			code |= mask;

			break;
		}
		case POP: {
			if (isImmediate(firstOperand)) {
				error("Destination addressing mode is immediate", true);
			}

			code |= mask;
			code <<= 5;

			break;
		}
		case CALL: {
			code <<= 5;
			code |= mask;

			break;
		}
		default: {
			error("Instruction processing error", true);

			break;
		}
		}


	}
	else {	// IMA DVA OPERANDA
		if (isImmediate(firstOperand)) {
			error("Destination addressing mode is immediate", true);
		}
		
		int dst = operandToMask(entry, section, firstOperand, additionalBytes);
		int* firstOperandAdditionalBytes = nullptr;	// da bi se upamtila vrednost
		if (additionalBytes) {
			firstOperandAdditionalBytes = new int[1];
			*firstOperandAdditionalBytes = *additionalBytes;
		}
		sizeBasedOnFirstOperand = entry->size;
		delete additionalBytes;
		additionalBytes = new int[1];
		*additionalBytes = 0;
		int src = operandToMask(entry, section, secondOperand, additionalBytes);
		if (!additionalBytes && firstOperandAdditionalBytes != nullptr) {
			additionalBytes = new int[1];
			*additionalBytes = *firstOperandAdditionalBytes;
		}
		delete firstOperandAdditionalBytes;

		switch (inst) {
		case ADD: case SUB: case MUL: case DIV: case CMP: case AND: case OR: case NOT: case TEST: case MOV: case SHL: case SHR: {
			code |= dst;
			code <<= 5;
			code |= src;





			break;
		}
		default: {
			error("Instruction processing error for: " + constInstTok + " " + firstOperand + " " + secondOperand, true);

			break;
		}

		}

	}


	if (sizeBasedOnFirstOperand == 4) {
		entry->size = 4;
	}
	else if (sizeBasedOnFirstOperand == -1) {
		entry->size = -1;
	}

	if ((entry->size == 4) && (additionalBytes != nullptr)) {
		code <<= 16;
		//*additionalBytes &= 0xFFFF;
		code |= *additionalBytes;
	}

	entry->value = code;
	delete additionalBytes;
	return entry->size;
}


int Assembler::operandToMask(Entry* entry, Section* section, string operand, int*& additionalBytes) {
	TokenType operandType = parseToken(operand);

	int mask = 0;

	switch (operandType) {
	case IMM: case IMM_HEX: {
		//mask |= 0;	// nema potrebe jer je 0
		//mask <<= 3;	// -||-

		if (operand[0] == '-') {
			stringstream(operand) >> *additionalBytes;
			*additionalBytes &= 0xFFFF;
		}
		else {
			*additionalBytes = stoi(operand, nullptr, 0);
		}

		entry->size = 4;

		break;
	}
	case PSW: {
		// kao i za neposredno adresiranje:
		//mask |= 0;
		//mask <<= 3;

		mask |= 7;
		delete additionalBytes;
		additionalBytes = nullptr;
		entry->size = 2;

		break;
	}
	case VALUE: {
		Symbol* s = processSymbol(entry, section, operand.substr(1), R_386_32);

		// kao i za neposredno adresiranje:
		//mask |= 0;
		//mask <<= 3;

		if (s) {
			*additionalBytes = s->offset;
			entry->size = 4;
		}
		else {
			delete additionalBytes;
			additionalBytes = nullptr;
			entry->size = -1;
		}

		break;
	}
	case MEMDIR: {
		mask |= 2;
		mask <<= 3;

		Symbol* s = processSymbol(entry, section, operand, R_386_32);
		if (s) {
			*additionalBytes = s->offset;
			entry->size = 4;
		}
		else {
			delete additionalBytes;
			additionalBytes = nullptr;
			entry->size = -1;
		}

		break;
	}
	case LOC: {
		mask |= 2;
		mask <<= 3;

		*additionalBytes = stoi(operand.substr(1), nullptr, 0);
		entry->size = 4;

		break;
	}
	case REGDIR: {
		mask |= 1;
		mask <<= 3;

		int reg = stoi(operand.substr(1), nullptr, 0);
		mask |= reg;

		delete additionalBytes;
		additionalBytes = nullptr;

		entry->size = 2;

		break;
	}
	case REGIND_DISP_IMM: {
		mask |= 3;
		mask <<= 3;

		int reg = stoi(operand.substr(1, 1), nullptr, 0);	// broj registra
		mask |= reg;

		string immVal = operand.substr(3);	// od prve cifre nadalje
		immVal.pop_back();	// uklanja zatvorenu uglastu zagradu
		*additionalBytes = stoi(immVal, nullptr, 0);
		entry->size = 4;

		break;
	}
	case REGIND_DISP_VAR: {
		mask |= 3;
		mask <<= 3;

		int reg = stoi(operand.substr(1, 1), nullptr, 0);	// broj registra
		mask |= reg;

		string symbol = operand.substr(3);	// od prve cifre nadalje
		symbol.pop_back();	// uklanja zatvorenu uglastu zagradu
		Symbol* s = processSymbol(entry, section, symbol, R_386_32);
		if (s) {
			*additionalBytes = s->offset;
			entry->size = 4;
		}
		else {
			delete additionalBytes;
			additionalBytes = nullptr;
			entry->size = -1;
		}

		break;
	}
	case PC_REL: {
		// kao i registarsko indirektno sa pomerajem (r7)
		mask |= 3;
		mask <<= 3;

		mask |= 7;	// r7 je PC registar

		Symbol* s = processSymbol(entry, section, operand.substr(1), R_386_PC32);
		if (s) {
			*additionalBytes = s->offset;
			entry->size = 4;
		}
		else {
			delete additionalBytes;
			additionalBytes = nullptr;
			entry->size = -1;
		}

		break;
	}
	case SYMBOL: {
		// kao PC relativno
		mask |= 3;
		mask <<= 3;

		mask |= 7;	// r7 je PC registar

		Symbol* s = processSymbol(entry, section, operand, R_386_PC32);
		if (s) {
			*additionalBytes = s->offset;
			entry->size = 4;
		}
		else {
			delete additionalBytes;
			additionalBytes = nullptr;
			entry->size = -1;
		}
		

		break;
	}
	default: {
		error("Operand processing error: " + operand, true);
		break;
	}
	}


	return mask;
}


Symbol* Assembler::processSymbol(Entry* entry, Section* section, string symbol, RelType relType) {
	if (symbol[0] == '#') {
		symbol = symbol.substr(1);
	}
	Symbol* s = findByName(symbol);
	if (s) {
		if (s->section == section->name) {
			return s;
		}
		else {
			Relocation r(section->name, entry->offset, relType, s->index);
			relocations.push_back(r);

			return nullptr;
		}
	}
	else {
		addSymbol(symbol, nullptr, -1, true);
		int index = symbolTable.size() - 1;
		Relocation r(section->name, entry->offset, relType, index);
		relocations.push_back(r);

		return nullptr;
	}
}


bool Assembler::isImmediate(string operand) {
	TokenType type = parseToken(operand);
	if (type == IMM || type == IMM_HEX || type == PSW ) {	// PSW se tretira kao neposredan, ne zahteva dodatne bajtove
		return true;
	}
	return false;
}


int Assembler::evaluateExpression(string expression) {
	char delimiter;
	if (expression.find('+') != string::npos) {
		delimiter = '+';
	}
	else {
		delimiter = '-';
	}
	
	string firstOperand = expression.substr(0, expression.find(delimiter));
	Symbol* s1 = findByName(firstOperand);
	if (!s1) {
		error("Unknown first operand: " + firstOperand + " in expression: " + expression, true);
	}

	string secondOperand = expression.substr(expression.find(delimiter) + 1);
	int val;
	TokenType type = parseToken(secondOperand);
	if (type == IMM || type == IMM_HEX) {
		val = stoi(secondOperand, nullptr, 0);
	}
	else if (type == SYMBOL) {
		Symbol* s2 = findByName(secondOperand);
		if (!s2) {
			error("Cannot find second operand (symbol): " + secondOperand + " in expression: " + expression, true);
		}
		val = s2->offset;
	}
	else {
		error("Unknown second operand: " + secondOperand + " in expression: " + expression, true);
	}

	return (delimiter == '+') ? (s1->offset + val) : (s1->offset - val);
}


void Assembler::print(ostream& ofs) {
	ofs << "SYMBOL TABLE" << endl << endl;
	ofs << "index" << '\t' << "name" << '\t' << '\t' << "section" << '\t' << "offset" << '\t' << "scope" << endl;
	ofs << "-----" << '\t' << "----" << '\t' << '\t' << "-------" << '\t' << "------" << '\t' << "-----" << endl;
	for (const Symbol& s : symbolTable) {
		ofs << s;
	}
	ofs << endl << endl << endl;



	printRelocationTable(ofs, Section::RODATA);
	ofs << *(Section::RODATA);


	printRelocationTable(ofs, Section::DATA);
	ofs << *(Section::DATA);
	

	printRelocationTable(ofs, Section::TEXT);
	ofs << *(Section::TEXT);

	
	printRelocationTable(ofs, Section::BSS);
	ofs << *(Section::BSS);



	ofs << "section" << '\t' << "address (hex)" << '\t' << "size" << "\t[FFFFFFFF as address means there is no section]" << endl;
	ofs << "-------" << '\t' << "-------------" << '\t' << '\t' << "----" << endl;
	ofs << Section::RODATA->name << '\t' << hex << Section::RODATA->startAddress << '\t' << '\t' << dec << Section::RODATA->size() << endl;
	ofs << Section::DATA->name << '\t' << hex << Section::DATA->startAddress << '\t' << '\t' << dec << Section::DATA->size() << endl;
	ofs << Section::TEXT->name << '\t' << hex << Section::TEXT->startAddress << '\t' << '\t' << dec << Section::TEXT->size() << endl;
	ofs << Section::BSS->name << '\t' << hex << Section::BSS->startAddress << '\t' << '\t' << dec << Section::BSS->size() << endl;



	if (errorList.size() > 0) {
		for (string& error : errorList) {
			cout << error << endl;
		}

		int x;
		cin >> x;
	}
}


void Assembler::printRelocationTable(ostream& ofs, const Section* s) {
	ofs << s->name << " section relocation table" << endl << endl;
	ofs << "offset" << '\t' << '\t' << "type" << '\t' << '\t' << "index" << endl;
	ofs << "------" << '\t' << '\t' << "----" << '\t' << '\t' << "-----" << endl;
	for (const Relocation& r : relocations) {
		if (r.section == s->name) {
			ofs << r;
		}
	}
	ofs << endl << endl << endl;
}