#ifndef Symbol_
#define Symbol_


#include <string>

class Symbol {

public:
	std::string name;
	std::string type;

	static int ID;

	int id;
	//int value;
	int sectionNumber;
	int offset;
	int lenght;
	char flag; 
	std::string flags;
	//vector<char> flags;
	
	Symbol(std::string nname, std::string ttype, int snumber,int ooffset,int llenght);
	Symbol(std::string nname, std::string ttype, int snumber, int ooffset, int llenght, char fflag);
	Symbol(std::string nname, std::string ttype, int snumber, int ooffset, int llenght, std::string fflags);
	~Symbol() {}

	void writeToDocument(std::ofstream& myFile)const;
	std::string getName();
	void setLenght(int l);
	void setOffset(int oofset) { offset = oofset; }
	void setFlag(char c) { flag = c; }

};

#endif // !Symbol_
