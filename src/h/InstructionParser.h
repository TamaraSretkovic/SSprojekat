#ifndef Instruction_Parser
#define Instruction_Parser

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
 
#define GLB_EXT 0
#define LABEL 1
#define SECTION 2
#define DIRECTIVE 3
#define SYMBOL 4
#define OPERAND_REG 5
#define OPERAND_REGSPEC 6
#define OPERAND_DEC 7
#define OPERAND_HEX 8
#define INSTRUCTION 9
#define SYMBOLDIFF 10

using namespace std;
class InstructionParser {
public: 

	
	vector<string> PCrelativeInstructions;// = { "int","jmp","call", "ret", "jeq", "jne", "jgt", "iret" };
	//string twoOperandInstruction[10] = {};  sve ostalo...
	vector<string> oneOperandInstruction;// = { "call","jgt","jne","jeq","jmp","pop","push","not","int" };
	vector<string> zeroOperandInstruction;// = { "halt","ret","iret" };


	InstructionParser();
	~InstructionParser();

	vector<string> getGlobalOrExtern(string line);
	string getSection(string line);
	string cutComment(string line);
	vector<string> getAddressingType(std::string line,bool word);
	vector<string> tokenizeInstruction(string line);
	string getInstructionName(string line);
	int getInstructionSize(vector<string> line,bool word);
	string cutLabel(string line);
	string cutName(string line);
	string getLabel(string line);
	bool isRelativeInstruction(string line);
	bool isWord(string instruction);
	int handleDirective(string line,int counter);

	bool isInstruction(string line);

	int numberOfOperands(string line);

	 unordered_map<string, int>* addressHash;
	 unordered_map<string, int>* instructionHash;
	 unordered_map<string, int>* registerHash;
	 unordered_map<int, std::regex>* tokenParsers;
};

#endif // !Instruction_Parser