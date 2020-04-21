#include "RelocationRecord.h"
#include <fstream>
#include <iomanip>

RelocationRecord::RelocationRecord(string ttype, int ooffset, int vvalue) {
	name = "";
	type = ttype;
	offset = ooffset;
	value = vvalue;
}
RelocationRecord::RelocationRecord(string nname) {
	name = nname;
	type = "";
	offset = 0;
	value=0;
}

void RelocationRecord::writeToDocument(ofstream& myfile) {
	if (name != "") {
		myfile << "\n";
		myfile << name << "\n";
		myfile << "Ofset   " << "    Tip" << "    vrednost  " << "\n";
	}
	else {
		myfile << hex << std::setw(8) << std::setfill('0') << offset << "    " << type  << "     " <<value<< "\n";
	}
}