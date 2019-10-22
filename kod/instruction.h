#pragma once

#include <string>
#include <vector>
#include <regex>
#include <unordered_map>


using namespace std;



enum TokenType { ILLEGAL, LABEL, GLOBAL, SECTION, DIRECTIVE, SYMBOL, IMM, IMM_HEX, PSW, VALUE, MEMDIR, 
	LOC, REGDIR, REGIND_DISP_IMM, REGIND_DISP_VAR, PC_REL, INSTRUCTION, END, EXPRESSION };

enum Operands { TWO_OPERANDS, ONE_OPERAND, NO_OPERANDS, ERROR };

enum InstructionCode { ADD = 0, SUB = 1, MUL = 2, DIV = 3, CMP = 4, AND = 5, OR = 6, NOT = 7, 
	TEST = 8, PUSH = 9, POP = 10, CALL = 11, IRET = 12, MOV = 13, SHL = 14, SHR = 15 };

enum ConditionCode { EQ = 0, NE = 1, GT = 2, AL = 3 };



class Instruction {
private:

public:
	
	static const unordered_map<int, regex> tokenRegexMap;
	static const unordered_map<int, regex> operandNumRegexMap;

	static bool isOperand(TokenType tokenType);
	static bool requiresFourBytes(TokenType instructionType);

	static bool isRet(string instructionToken);
	static bool isJmp(string instructionToken);
	static bool isCall(string instructionToken);

	static const unordered_map<string, InstructionCode> instructionCodes;
	static const unordered_map<string, ConditionCode> conditionCodes;

	static InstructionCode getInstruction(string instructionToken);
	static ConditionCode getCondition(string instructionToken);

};