#include "relocation.h"

#include <iostream>
#include <iomanip>


ostream& operator<<(ostream& os, const Relocation& r) {
	os << hex << uppercase << setw(8) << setfill('0') << r.offset << '\t' << (r.relType == R_386_32 ? "R_386_32" : "R_386_PC32") << '\t' << dec << r.index << endl;
	return os;
}