
#ifndef Relocation_Record
#define Relocation_Recosd
#include <string>

using namespace std;
class RelocationRecord {
public:
	std::string name;
	string type;
	int offset;
	int value;

	RelocationRecord(string ttype, int ooffset, int vvalue);
	RelocationRecord(string nname);

	void writeToDocument(ofstream& myfile);
};
#endif // !Relocation_Record
