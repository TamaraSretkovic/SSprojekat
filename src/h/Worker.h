#ifndef Worker_
#define Worker_

#include <fstream>
#include <sstream>
#include "Symbol.h"
#include <list>
#include <vector>
#include "RelocationRecord.h"
#include "InstructionParser.h"

using namespace std;

	class Worker
	{
	public:

		InstructionParser* instructionParser;

		 list<Symbol > listOfSymbols;
		 list<RelocationRecord> listOfRecs;
		vector<string> externSymbols;
		 unordered_map<string, int>* equList;

		string currentSection;
		int locationCounter;
		int currentSectionNumber;
		string currentFlags;

		 ofstream myfile;
		 ifstream infile;

		char flag;

		bool relative;
		bool pcRelative;

		Worker(string in, string out);
		~Worker();

		void firstPass();
		void secondPass();
		//void writeToFile();

		void readInstructions(string line); //done sta je ovde done???
		int relocation(string label,bool relative, bool pcRelative, int offset);

		short int reverseBytes16(short int value); //done
		
		void handleDirectiveSecondPass(string line);
		void operate(vector<string> addrType, vector<string> tokens,int num, bool isWord, string line);
		
		bool findGlb(string name);

	};

#endif // !Worker



