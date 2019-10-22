#pragma once

#include <string>
#include <iostream>

#include "section.h"


using namespace std;


class Symbol {
public:
	int index;
	string name;
	// Section* section;
	string section;
	int offset;
	bool isGlobal;

	Symbol(int i, string n, /* Section* */ string s, int o, bool g) {
		index = i;
		name = n;
		section = s;
		offset = o;
		isGlobal = g;
	}

	friend ostream& operator<<(ostream& os, const Symbol& s);

};

