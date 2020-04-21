#include "Symbol.h"
#include "fstream"

int Symbol::ID = 0;

Symbol::Symbol(std::string nname, std::string ttype, int snumber, int ooffset, int llenght) {
	name=nname;
	type = ttype;
	sectionNumber = snumber;
	offset = ooffset;
	lenght = llenght;
	id = ID++;
	flag='u';
	if(name=="text")flags="xpa";
	else if(name=="bss")flags="pa";
	else if(name=="data")flags="wpa";
	else flags="";
}


Symbol::Symbol(std::string nname, std::string ttype, int snumber, int ooffset, int llenght, char fflag) {
	name = nname;
	flag = fflag;
	type = ttype;
	sectionNumber = snumber;
	offset = ooffset;
	lenght = llenght;
	id = ID++;
	flags="";
}

Symbol::Symbol(std::string nname, std::string ttype, int snumber, int ooffset, int llenght, std::string fflags) {
	name = nname;
	flag = 'u';
	type = ttype;
	sectionNumber = snumber;
	offset = ooffset;
	lenght = llenght;
	id = ID++;
	flags=fflags;
}
std::string Symbol::getName() { return name; }
void Symbol::setLenght(int l) { lenght = l; }

void Symbol::writeToDocument(std::ofstream& myfile) const{
	std::string padding;
	int difference = 10 - this->name.length();
	if (difference > 0) {
		for (int i = 0; i < difference; i++) padding = padding + " ";
	}
	myfile << this->type << "      " << this->id << "   " << this->name << padding << "     " << this->sectionNumber << "      " << this->offset << "      " << this->lenght << "      " << this->flag << "\n";
}
