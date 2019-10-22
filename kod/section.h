#pragma once

#include <string>
#include <vector>
#include <iostream>



using namespace std;



struct Entry {
	int offset;
	int value;
	int size;
};


class Section {
private:
	bool firstAppearance = true;

	vector<Entry> entries;

public:
	static Section* const TEXT;
	static Section* const DATA;
	static Section* const RODATA;
	static Section* const BSS;

	//int locationCounter = 0;

	const string name;

	Section(const string n);

	bool checkIfFirstAppearance();

	void addEntry(Entry entry);

	int startAddress = -1;
	int size();

	friend ostream& operator<<(ostream& os, const Section& s);

};