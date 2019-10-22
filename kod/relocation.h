#pragma once

#include <string>
#include "section.h"


using namespace std;


enum RelType { R_386_32, R_386_PC32 };


class Relocation {
public:
	string section;
	int offset;
	RelType relType;
	int index;

	Relocation(string s, int o, RelType r, int i) {
		section = s;
		offset = o;
		relType = r;
		index = i;
	}

	friend ostream& operator<<(ostream& os, const Relocation& r);

};