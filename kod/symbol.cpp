#include <iostream>
#include <iomanip>

#include "symbol.h"


using namespace std;



ostream& operator<<(ostream& os, const Symbol& s) {
	os << s.index << '\t' << s.name << '\t' << '\t' << s.section << '\t';
	if (s.offset >= 0) {
		os << s.offset;
	}
	else {
		os << '?';
	}
	os << '\t' << (s.isGlobal ? "global" : "local") << endl;
	return os;
}