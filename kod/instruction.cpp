#include "instruction.h"


const unordered_map<int, regex> Instruction::tokenRegexMap = {
	{ SYMBOL, regex("^[a-zA-Z_][a-zA-Z0-9]*$") },
	{ LABEL, regex("^([a-zA-Z_][a-zA-Z0-9]*):$") },
	{ GLOBAL, regex("^\\.globa?l$") },
	{ SECTION, regex("^\\.(text|data|rodata|bss)$") },
	{ DIRECTIVE, regex("^\\.(char|word|long|align|skip)$") },
	{ IMM, regex("^-?[0-9]+$") },
	{ IMM_HEX, regex("^0x[0-9abcdefABCDEF]+$") },
	{ PSW, regex("^psw$") },
	{ VALUE, regex("^&[a-zA-Z_][a-zA-Z0-9]*$") },
	{ MEMDIR, regex("^#[a-zA-Z_][a-zA-Z0-9]*$") },
	{ LOC, regex("^\\*[0-9]+$") },
	{ REGDIR, regex("^r[0-7]$") },
	{ REGIND_DISP_IMM, regex("^r[0-7]\\[[0-9]+\\]$") },
	{ REGIND_DISP_VAR, regex("^r[0-7]\\[[a-zA-Z_][a-zA-Z0-9]*\\]$") },
	{ PC_REL, regex("^\\$[a-zA-Z_][a-zA-Z0-9]*$") },
	{ INSTRUCTION, regex("^(add|sub|mul|div|cmp|and|or|not|test|push|pop|call|iret|mov|shl|shr|ret|jmp)(eq|ne|gt|al)?$") },
	{ END, regex("^\\.end$") },
	{ EXPRESSION, regex("^[a-zA-Z_][a-zA-Z0-9]*(\\+|-)([a-zA-Z_][a-zA-Z0-9]*|[0-9]+)$") }
};


const unordered_map<int, regex> Instruction::operandNumRegexMap = {
	{ TWO_OPERANDS, regex("^(add|sub|mul|div|cmp|and|or|not|test|mov|shl|shr)(eq|ne|gt|al)?$") },
	{ ONE_OPERAND, regex("^(push|pop|call|jmp)(eq|ne|gt|al)?$") },
	{ NO_OPERANDS, regex("^(iret|ret)(eq|ne|gt|al)?$") }
};


const unordered_map<string, InstructionCode> Instruction::instructionCodes = {
	{ "add",	InstructionCode::ADD },
	{ "sub",	InstructionCode::SUB },
	{ "mul",	InstructionCode::MUL },
	{ "div",	InstructionCode::DIV },
	{ "cmp",	InstructionCode::CMP },
	{ "and",	InstructionCode::AND },
	{ "or",		InstructionCode::OR },
	{ "not",	InstructionCode::NOT },
	{ "test",	InstructionCode::TEST },
	{ "push",	InstructionCode::PUSH },
	{ "pop",	InstructionCode::POP },
	{ "call",	InstructionCode::CALL },
	{ "iret",	InstructionCode::IRET },
	{ "mov",	InstructionCode::MOV },
	{ "shl",	InstructionCode::SHL },
	{ "shr",	InstructionCode::SHR }
	// ret i jmp su pseudoinstrukcije
};


const unordered_map<string, ConditionCode> Instruction::conditionCodes = {
	{ "eq", ConditionCode::EQ },
	{ "ne", ConditionCode::NE },
	{ "gt", ConditionCode::GT },
	{ "al", ConditionCode::AL }
};


bool Instruction::isOperand(TokenType tokenType) {
	if (tokenType == IMM || tokenType == IMM_HEX || tokenType == PSW || tokenType == VALUE || tokenType == MEMDIR || tokenType == LOC
		|| tokenType == REGDIR || tokenType == REGIND_DISP_IMM || tokenType == REGIND_DISP_VAR || tokenType == PC_REL
		|| tokenType == SYMBOL) {	// STA ZA SYMBOL?
		return true;
	}
	return false;
}

bool Instruction::requiresFourBytes(TokenType instructionType) {
	if (instructionType == IMM || instructionType == IMM_HEX || instructionType == MEMDIR || instructionType == LOC
		|| instructionType == REGIND_DISP_IMM || instructionType == REGIND_DISP_VAR || instructionType == PC_REL
		|| instructionType == SYMBOL) {	// STA ZA SYMBOL?
		return true;
	}
	return false;
}


InstructionCode Instruction::getInstruction(string instructionToken) {
	for (auto& pair : instructionCodes) {
		regex r("^" + pair.first + "(eq|ne|gt|al)?$");
		if (regex_match(instructionToken, r)) {
			return pair.second;
		}
	}

	// RETURN U SLUCAJU NENALAZENJA?
}


ConditionCode Instruction::getCondition(string instructionToken) {
	for (auto& pair : conditionCodes) {
		regex r("^(add|sub|mul|div|cmp|and|or|not|test|push|pop|call|iret|mov|shl|shr|ret|jmp)" + pair.first + "$");
		if (regex_match(instructionToken, r)) {
			return pair.second;
		}
	}

	return AL;
}


bool Instruction::isRet(string instructionToken) {
	return regex_match(instructionToken, regex("^ret(eq|ne|gt|al)?$"));
}



bool Instruction::isJmp(string instructionToken) {
	return regex_match(instructionToken, regex("^jmp(eq|ne|gt|al)?$"));
}


bool Instruction::isCall(string instructionToken) {
	return regex_match(instructionToken, regex("^call(eq|ne|gt|al)?$"));
}