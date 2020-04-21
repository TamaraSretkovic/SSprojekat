#include "InstructionParser.h"
#include<unordered_map>
#include <string>
#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>
#include "Worker.h"
// #include <boost/algorithm/string.hpp>    ZASTO NEMA????!!!!


using namespace std;


bool InstructionParser::isRelativeInstruction(std::string line) {
	bool result = false;
	int a = PCrelativeInstructions.size();
	for (int i = 0; i < a; i++) {
		if (line == PCrelativeInstructions[i]) {
			result = true;
			return result;
		}
	}
	//if (line.find("$") != string::npos)result = true; RAZMISLIII!!!-> NIJE NE MENJA PC
	return result;
}

InstructionParser::InstructionParser()
{

	 PCrelativeInstructions={ "int","jmp","call", "ret", "jeq", "jne", "jgt", "iret" };
	oneOperandInstruction = { "call","jgt","jne","jeq","jmp","pop","push","not","int" };
	zeroOperandInstruction = { "halt","ret","iret" };


	addressHash = new unordered_map<string, int>();
	instructionHash = new unordered_map<string, int>();
	registerHash = new unordered_map<string, int>();
	tokenParsers = new unordered_map<int, regex>();

	tokenParsers->insert(pair<int, regex>(GLB_EXT, regex("^(\\.global|\\.extern)")));   //trebace
	tokenParsers->insert(pair<int, regex>(LABEL, regex("^([a-zA-Z_][a-zA-Z0-9]*):")));  //da li? da za izdvajanje labele? 
	tokenParsers->insert(pair<int, regex>(SECTION, regex("\\.(text|data|bss|section)( [a-zA-Z_][a-z A-Z0-9]*,?)?")));  //ovo mora da se prepravi
	tokenParsers->insert(pair<int, regex>(DIRECTIVE, regex("\\.(byte|word|align|skip)")));  //ovo je ok, DODAJ KOD ZA EQU
	tokenParsers->insert(pair<int, regex>(SYMBOL, regex("^([a-zA-Z_][a-zA-Z0-9]*)$"))); //ovo moze u adresiranju da bude
	tokenParsers->insert(pair<int, regex>(OPERAND_REG, regex("r([0-7f])(l|h)?")));  //nadji registar
	tokenParsers->insert(pair<int, regex>(OPERAND_DEC, regex("^([0-9]+)$")));  //nadji broj imm
	tokenParsers->insert(pair<int, regex>(OPERAND_HEX, regex("^(0x[0-9abcdef]+)$")));  //nadji 0x0
	tokenParsers->insert(pair<int, regex>(INSTRUCTION, regex("(int|add|sub|mul|mov|div|cmp|and|or|not|test|xor|xchg|call|shr|shl|halt|ret|iret|pop|push|jmp|jeq|jne|jgt)(b|w)?")));
	//NE RADI ZA INSTRUKCJE????
	tokenParsers->insert(pair<int, regex>(SYMBOLDIFF, regex("^([a-zA-Z_][a-zA-Z0-9]*)-([a-zA-Z_][a-zA-Z0-9]*)$"))); //da li treba??

	addressHash->insert(pair<string, int>("imm",0));
	addressHash->insert(pair<string, int>("regDir", 1));
	addressHash->insert(pair<string, int>("regIndir", 2));
	addressHash->insert(pair<string, int>("regIndir8Pom", 3));
	addressHash->insert(pair<string, int>("regIndir16Pom", 4));
	addressHash->insert(pair<string, int>("memDir", 5));


	
	registerHash->insert(pair<string, int>("r0",0));
	registerHash->insert(pair<string, int>("r1", 1));
	registerHash->insert(pair<string, int>("r2", 2));
	registerHash->insert(pair<string, int>("r3", 3));
	registerHash->insert(pair<string, int>("r4", 4));
	registerHash->insert(pair<string, int>("r5", 5));
	registerHash->insert(pair<string, int>("r6", 6));
	registerHash->insert(pair<string, int>("r7", 7));
	registerHash->insert(pair<string, int>("rf", 15)); //r3..r0



	///////////////////KAKO DA NE BUDE CASE SENSITIVE?! //////////////////////////////////

	instructionHash->insert(pair<string, int>("halt", 1));

	instructionHash->insert(pair<string, int>("xchg", 2));
	instructionHash->insert(pair<string, int>("int", 3));
	instructionHash->insert(pair<string, int>("mov", 4));
	instructionHash->insert(pair<string, int>("add", 5));
	instructionHash->insert(pair<string, int>("sub", 6));
	instructionHash->insert(pair<string, int>("mul", 7));
	instructionHash->insert(pair<string, int>("div", 8));

	instructionHash->insert(pair<string, int>("cmp", 9));
	instructionHash->insert(pair<string, int>("not", 10));
	instructionHash->insert(pair<string, int>("and", 11));
	instructionHash->insert(pair<string, int>("or", 12));
	instructionHash->insert(pair<string, int>("xor", 13));  
	instructionHash->insert(pair<string, int>("test", 14));
	instructionHash->insert(pair<string, int>("shl", 15));
	instructionHash->insert(pair<string, int>("shr", 16));

	instructionHash->insert(pair<string, int>("push", 17));
	instructionHash->insert(pair<string, int>("pop", 18));

	instructionHash->insert(pair<string, int>("jmp", 19));
	instructionHash->insert(pair<string, int>("jeq", 20));
	instructionHash->insert(pair<string, int>("jne", 21));
	instructionHash->insert(pair<string, int>("jgt", 22));
	instructionHash->insert(pair<string, int>("call", 23));
	instructionHash->insert(pair<string, int>("ret", 24));
	instructionHash->insert(pair<string, int>("iret", 25));
	 
	
	
}

int InstructionParser::handleDirective(string line, int counter) {
	smatch s;
	if (regex_search(line, s, tokenParsers->at(DIRECTIVE))) { // NI OVDE NE PRONALAZI???
		string directive = s[1];
		int ret = 0;
		line = cutLabel(line);
		std::string segment;
		std::stringstream ss(line);
		while (std::getline(ss, segment, ','))ret++;

		if (directive == "byte")return ret;
		if (directive == "word")return 2 * ret;
		if (directive == "skip") {
			int pom = line.find_first_of(".");
			pom += 6;
			string align = line.substr(pom, line.length());
			
			int i=0;
			if (line.find_first_of("x") == string::npos)
				i = std::stoi(align, nullptr, 10); //SAMO ZA DECIMALNO PORAVNANJE
			else
				i = std::stoi(align, nullptr, 16);
			return i;
		}
		if (directive == "align") {
			int pom = line.find_first_of(".");
			pom += 7;
			string align = line.substr(pom, line.length());
			int i;
			if(line.find_first_of("x")==string::npos)
			i = std::stoi(align, nullptr, 10); //SAMO ZA DECIMALNO PORAVNANJE
			else
			 i = std::stoi(align, nullptr, 16); //ZA HEXA PORAVNANJE
			ret = counter;
			while (ret % i != 0)ret++;
			return ret-counter; //vrati razliku za koju si uvecala counter
		}
	}

		return -1;
}

vector<string> InstructionParser::getGlobalOrExtern(string line) {
	line=cutComment(line);
	smatch s;
	vector<std::string> seglist;
	if (regex_search(line, s, tokenParsers->at(GLB_EXT))) {
		std::string segment;
		int start = line.find_first_of(".");
		start += 7;
		line = line.substr(start, line.length());
		std::stringstream ss(line);
		while (std::getline(ss, segment, ',')) {
			int start = segment.find_first_not_of(" ");
			segment = segment.substr(start, segment.length());
			start = segment.find_first_of(" ");
			segment = segment.substr(0, start); //delete blanko
			seglist.push_back(segment);
		}

	}
	return seglist;
}

string InstructionParser::getInstructionName(std::string line)
{
	line = cutLabel(line);
	if (line.find_first_not_of("	")==string::npos) return "labelOnly";
	if (line.find_first_not_of(" ") == string::npos) return "labelOnly";
	int start = line.find_first_not_of(" ");  // dodji do pocetnog slova instr
	line = line.substr(start, line.size());
	
	int end = line.find_first_of(" ");   //posle instr ide blanko znak
	line = line.substr(0, end);

	smatch s;
	if (regex_match(line, s, tokenParsers->at(INSTRUCTION))) {
		line = s[1];
	}

	return line;
}

string InstructionParser::cutComment(string line) {
	int endpos = line.find_first_of("#");
	if(endpos!=string::npos) line = line.substr(0, endpos -1);
	return line;
}

string InstructionParser::cutLabel(std::string line) {
	int labelPos = line.find_first_of(":");
	if (labelPos != string::npos) line = line.substr(labelPos + 1, line.size());
	return line;
}

string InstructionParser::cutName(std::string line) {
	int namePos = line.find_first_not_of(" "); //obrisi spejseve
	if (namePos != string::npos) line = line.substr(namePos, line.size());

	int endPos = line.find_first_of(" ");
	if (endPos != string::npos) line = line.substr(endPos, line.size());
	else { endPos = line.find_first_of("	"); 
	if (endPos != string::npos) line = line.substr(endPos, line.size());
	else line = "";
	}

	//int start = line.find_first_not_of(" ");
	//if (start != string::npos) line = line.substr(start, line.size());

	/*//boots::to_lower(line);
	std::for_each(line.begin(), line.end(), [](char& c) {
		c = ::tolower(c);
	});   // jedino ovako?
	*/
	return line;
}

int InstructionParser::getInstructionSize(vector<std::string> adrType, bool word) {

	//PROVERI GDE CES DA PROVERIS DA LI ADRESIRANJA ODGOVARAJU INSTRUKCIJI!!!

	int size = 1;
	if (adrType[0] == "zero")return size;
	if (adrType[0] == "regDir" || adrType[0] == "regIndir") size += 1;
	else if (adrType[0] == "regIndir16Pom"  || adrType[0] == "imm" || adrType[0] == "memDir") size += 3;
	else if (adrType[0] == "regIndir8Pom")size += 2;
	else return -1;
	if (adrType.size() == 2) {
		if (adrType.at(1) == "regDir" || adrType[1] == "regIndir") size += 1;
		else if (adrType[1] == "regIndir16Pom" || adrType[1] == "imm" || adrType[1] == "memDir") size += 3;
		else if (adrType[1] == "regIndir8Pom")size += 2;
		else return -1;
	}
	if (word)return size;
	if (adrType[0] == "imm")size--;
	if (adrType.size() == 2 && adrType[1] == "imm")size--;
	return size; //AKO SU OPERANDI BAJTOVSKI I AKO JE IMM ADRESIRANJE ODUZMI 1 B!!!!
}
//STA AKO INS NEMAJU OPERANDE??? PROVERI ZA POP< PUSH< RET< IRET< HALT///

InstructionParser::~InstructionParser()
{
}

bool InstructionParser::isWord(string name) {
	smatch s;
	if (regex_search(name, s, tokenParsers->at(INSTRUCTION))) { //zasto ne radi??? NE RASI NI SA NI BES $
		auto ss = s[2];
		if (ss=="w") return true;
		else if ( ss=="b")return false;
	}
	return true; //podrazumevano word
}

string InstructionParser::getLabel(string line) {
	smatch s;
	if (regex_search(line, s, tokenParsers->at(LABEL))) {
		line=s[1];
	}
	else line="";
	return line;
}

string InstructionParser::getSection(string line) {
	smatch s;
	string section="";
	if (regex_search(line, s, tokenParsers->at(SECTION))) {
		section = s[1];

			if (section=="section") {
			int start = line.find_first_of(".");
			start += 9;
			line=line.substr(start,line.length());
			int end = line.find_first_of(",");

			if(end!=string::npos){
			section = line.substr(0,end);}
			 else {section = line.substr(start, line.length()); } // ako je .section ime_sekcije
		}
	}
	return section;
}

vector<string> InstructionParser::getAddressingType(std::string line,bool word) {
	line = cutComment(line);
	line = cutLabel(line);
	smatch s;
	
	
	std::string segment;
	vector<std::string> seglist;
	bool hasRegister = false;
	vector<string> addType;
	if (regex_search(line, s, std::regex("(ret|halt|iret)"))) {
		addType.push_back("zero");
		line = cutName(line);
		//prvi ili drugi prolaz???
		//if (line != "" || line != " ") throw runtime_error("ret, halt and iret doesnt have operands");
	}
	else {
		//ako je samo instrukcija
		line = cutName(line);
		std::stringstream ss(line);
		while (std::getline(ss, segment, ',')) seglist.push_back(segment);   //ako bude 2 operanda-> 2 elementa u seglisti


		for (unsigned int i = 0; i < seglist.size(); i++) {
			hasRegister = false;
			std::string curr = seglist.at(i);
			smatch s;

			if (regex_search(curr, s, tokenParsers->at(OPERAND_REG)))
				hasRegister = true;



			if (hasRegister && curr.find("[") != string::npos) {
				int p1 = curr.find("[");
				int p2 = curr.find("]");
				if (word && (p2 - p1) == 1)
					addType.push_back("regIndir");
				else if (word && (p2 - p1) != 1)addType.push_back("regIndir16Pom");
				else if(!word)addType.push_back("regIndir8Pom");
				
				continue;
				//Reg Ind e sad van ovoga proveri kakav je operand
			}
			else if (hasRegister) {
				addType.push_back("regDir");
				continue;
			}												//regDir

			if (curr.find("$") != string::npos) {
				if (word)
					addType.push_back("regIndir16Pom");
				else addType.push_back("regIndir8Pom");
				continue;
			}                                            //PC relativno


			if (curr.find("*") != string::npos) {
				addType.push_back("memDir");
				continue;
			}											//memdir

			if (curr.find("&") != string::npos) {
				addType.push_back("imm");
				continue;
			}                                          //imm


			if (regex_match(curr, s, tokenParsers->at(OPERAND_DEC)) || regex_match(curr, s, tokenParsers->at(OPERAND_HEX))) {//ako je broj ali razmisli sta ako je -???
				addType.push_back("imm");
				continue;
			}
			else addType.push_back("memDir");  


		}
	}
	return addType;
	}
	
vector<string> InstructionParser::tokenizeInstruction(string line)
{

	//PROVERI ZA OZNAKE ZA ADRESIRANJA, VRLO JE PIPLJIVO I BEZVEZE...:/


	line = cutLabel(line);
	line = cutName(line);
	std::string segment;
	vector<std::string> seglist;
	int start=0;
	std::stringstream ss(line);
	while (std::getline(ss, segment, ','))
	{
		int pos;
		pos = segment.find("[");
		if (pos != string::npos) {					//ako nema [, nije regind probaj nesto drugo
			/*string reg = segment.substr(0, pos-1);

			segment.erase(segment.begin() + pos);
			int pos2 = segment.find("]");
			string pom = segment.substr(0,pos2); //zasto ispisuje i ] ??? RESENO
			segment.erase(0,pos2); 
			seglist.push_back(reg);
			if (pom != "")seglist.push_back(pom);; // ako nema pomeraja npr R3[]
			
*/
			//ako ima zbir ili razlika ???? moras da imas oba operanda za 2.proalaz!!!
			
			string reg= segment.substr(0,pos);
			int pom2= reg.find_first_not_of(" ");
			if(pom2!=string::npos)reg=reg.substr(pom2,reg.length());
			seglist.push_back(reg);

			pom2=segment.find("]");
			string operand=segment.substr(pos+1,pom2);
			if(operand!="")seglist.push_back(operand);		
			continue;
			
		}
		else {
			pos = segment.find("$");				//pc relativno
			if (pos != string::npos) { 
				segment.erase(segment.begin() + pos);
				seglist.push_back("r7");
			}
			else {
			pos= segment.find("&");					//neposredno adr simbola
			if (pos != string::npos) segment.erase(segment.begin() + pos);
			else {
				pos = segment.find("*");				//memdir
				if (pos != string::npos) segment.erase(segment.begin() + pos);
			     }
			}
		}
		int pom2= segment.find_first_not_of(" ");
		segment=segment.substr(pom2,segment.length());
		seglist.push_back(segment);  //ostaje samo labela ili broj ili r5l? r3h? 
	}
	return seglist;
}

bool InstructionParser::isInstruction(string line) {
	smatch s;
	if (regex_search(line, s, tokenParsers->at(INSTRUCTION)))return true;
	return false;
}

int InstructionParser::numberOfOperands(string line) {
	smatch s;
	string name="";
	if (regex_search(line, s, tokenParsers->at(INSTRUCTION))) name=s[1];
	if(name=="")return -1;
	for( unsigned i=0;i<oneOperandInstruction.size();i++)
		if(oneOperandInstruction[i]==name) return 1;
	for (unsigned i = 0; i < zeroOperandInstruction.size(); i++)
		if (zeroOperandInstruction[i] == name) return 0;

	return 2;

}
