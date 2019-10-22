#include "section.h"

#include <iomanip>
#include <bitset>


Section* const Section::TEXT = new Section("TEXT");
Section* const Section::DATA = new Section("DATA");
Section* const Section::RODATA = new Section("RODATA");
Section* const Section::BSS = new Section("BSS");


Section::Section(const string n) : name(n) { }


bool Section::checkIfFirstAppearance() {
	if (firstAppearance) {
		firstAppearance = false;
		return true;
	}
	else {
		return false;
	}
}


void Section::addEntry(Entry entry) {
	entries.push_back(entry);
}


int Section::size() {
	if (entries.size() > 0) {
		Entry& e = entries.back();
		return e.offset + e.size;
	}
	return 0;
}


ostream& operator<<(ostream& os, const Section& s) {
	os << s.name << endl << endl;
	os << right;	// treba da bude right zbog setfill ispod
	for (const Entry& entry : s.entries) {
		os << entry.offset << '\t';
		if (entry.size > 0) {
			for (int i = entry.size - 1; i >= 0; i--) {
				int val = entry.value;
				val >>= (i % 4) * 8;
				val &= 0xFF;
				os << hex << uppercase << setw(2) << setfill('0') << val << ' ';
			}
			os << '\t';
			if (entry.size <= 3) {
				os << '\t';
			}
			for (int i = entry.size - 1; i >= 0; i--) {
				int val = entry.value;
				val >>= (i % 4) * 8;
				val &= 0xFF;
				bitset<8> bits(val);
				os << bits << ' ';
			}
		}
		else /*if (entry.size == -1)*/ {
			for (int i = 1; i >= 0; i--) {
				int val = entry.value;
				val >>= (i % 4) * 8;
				val &= 0xFF;
				os << hex << uppercase << setw(2) << setfill('0') << val << ' ';
			}
			os << "?? ?? ";
			os << '\t';
			for (int i = 1; i >= 0; i--) {
				int val = entry.value;
				val >>= (i % 4) * 8;
				val &= 0xFF;
				bitset<8> bits(val);
				os << bits << ' ';
			}
			os << "???????? ???????? ";
		}
		os << endl;
	}
	os << endl << endl << endl;

	return os;
}