#include "Worker.h"
#include <string>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <stdlib.h>
#include <ctype.h>
#include <iomanip>
#include <list>
#include <iostream>

using namespace std;


bool Worker::findGlb(string name) {
	for (unsigned i = 0; i < externSymbols.size(); i++) {
		if (name == externSymbols[i])return true;
	}return false;
}

void Worker::firstPass() {

	std::string line;
	getline(infile, line);
	std::string readedContent = "";

	while (line.find(".end")) {
		readedContent = "";
		if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1); //PRAZNA ili ENTER?

		if (line.empty()) {				//PRANZA
			getline(infile, line);
			continue;
		}

		if (line.find(".global") != string::npos) {
			getline(infile, line);
			continue;
		}
		vector<string> glb = instructionParser->getGlobalOrExtern(line);
		if (glb.size() > 0) {
			getline(infile, line);
			continue; //posle .global ili .extern nema nista..
		}
		if(line.find(".equ")!=string::npos){
				regex r("([a-zA-Z_][a-zA-Z0-9] *)");
				regex rr("([0-9]+)");
				smatch s1,s2;
				int pos=line.find_first_of(".");
				pos+=5;
				line=line.substr(pos,line.length());
				pos=line.find_first_of(",");
				if(pos!=string::npos){
					string variable=line.substr(0,pos);
					string value=line.substr(pos+1,line.length());
					//if (regex_search(variable, s1, r))
					//if (regex_search(value, s2, rr))
						equList->insert(pair<string, int>(variable, stoi(value)));
				}
				else throw runtime_error("Directive .equ must be followed by label ',' and number");
				
			getline(infile, line);
			continue;
		}
		//AKO NE POCINJE SEKCIJOM
		if ((readedContent = instructionParser->getSection(line)) == "" && currentSection == "")throw runtime_error("no starting section");
		else {
			if (readedContent != "") {
				if (currentSection != "")
					for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
						if (ci->Symbol::name == currentSection) {
							ci->Symbol::lenght = locationCounter;
						}// POSTAVLJANJE DUZINE TRENUTNE SEKCIJE
						if (ci->Symbol::name == readedContent) throw runtime_error("one section only can be defined only once");
					}
				currentSection = readedContent;
				locationCounter = 0;
				currentSectionNumber++;
				if(line.find(".section")!=string::npos){
					int pos=line.find_first_of(",");
					if(pos!=string::npos){
						string f=line.substr(pos,line.length());
						listOfSymbols.push_back(Symbol(readedContent, "SECTION", currentSectionNumber, locationCounter, 0,f));
					}else listOfSymbols.push_back(Symbol(readedContent, "SECTION", currentSectionNumber, locationCounter, 0));
				}
				else
				listOfSymbols.push_back(Symbol(readedContent, "SECTION", currentSectionNumber, locationCounter, 0));

				getline(infile, line);
				continue;
			}
		}


		//da li ima labela?
		if ((readedContent = instructionParser->getLabel(line)) != "") { //AKO NE POSTOJI U TABELI SIMBOLA!
			//bool find = false;
			for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
				if (ci->Symbol::name == readedContent) {
					//find = true; break; // zar ne treba: 
					throw runtime_error("Label can not be defined twice");
				}
			}
			char flag = 'l'; //u drugom prolazu azuriraj
			/*if (find == false) {

				if (std::find(globalSymbols.begin(), globalSymbols.end(), readedContent) != globalSymbols.end()) flag = 'g'; // da li je ima u listi globalnih?
			}*/  //ovo u drugom krugu!!
			listOfSymbols.push_back(Symbol(readedContent, "LABEL", currentSectionNumber, locationCounter, 0, flag)); //da li labela ima duzinu?
		}


		if (instructionParser->getInstructionName(line) == "labelOnly") {
			getline(infile, line);
			continue;
		} //i da nije instrukcija moze da obavi posao



		//AKO IMA DIREKTIVE, moras da dodas vrednosti u data ? drugi prolaz?
		int size = instructionParser->handleDirective(line, locationCounter);
		if (size != -1) {
			locationCounter += size;
			getline(infile, line);
			continue;
		}

		if (instructionParser->isInstruction(line)) {
			vector <string> addrType = instructionParser->getAddressingType(line, instructionParser->isWord(line));
			if (addrType.size() > 0) {
				int size = instructionParser->getInstructionSize(addrType, instructionParser->isWord(line));
				locationCounter += size;
				getline(infile, line);
				continue;
			}
		}
		else throw runtime_error("Unknown comand.");
	}



	for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
		if (ci->Symbol::name == currentSection) {
			ci->Symbol::lenght = locationCounter;
		}
	}// POSTAVLJANJE DUZINE TRENUTNE SEKCIJE
	
}

Worker::Worker(string in, string out) {
	infile.clear();
	infile.seekg(0, ios::beg);

	locationCounter = 0;
	currentSection = "";
	flag = 'u';
	currentSectionNumber = -1;

	myfile.open(out, std::ofstream::out);
	infile.open(in, std::ifstream::out);   //IN? da li treba neka inicijalizacija???

	listOfSymbols = list<Symbol>();
	listOfRecs = list<RelocationRecord>();
	equList = new unordered_map<string, int>();

	relative = false;
	pcRelative = false;
	currentFlags="";

	instructionParser = new InstructionParser();

}

Worker::~Worker() {
	myfile.close();
	infile.close();
	listOfSymbols.clear();
	listOfRecs.clear();
	externSymbols.clear();
}

short int Worker::reverseBytes16(short int value) {
	short int swapped;
	swapped = ((value >> 8) & 0x00ff | (value << 8) & 0xff00);
	return swapped;
}

void Worker::secondPass() {
	//myfile << "\n \n \n";
	myfile << "-----------------#Sadrzaji sekcija--------------------" << "\n";
	infile.clear();
	infile.seekg(0, infile.beg);

	currentSection = "";
	currentSectionNumber = -1;
	flag = 'u';

	string line;
	getline(infile, line);
	string readedContent = "";

	while (line.find(".end")) {
		readedContent = "";
		if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1); //PRAZNA ili ENTER?

		if (line.empty()) {				//PRAZNA
			getline(infile, line);
			continue;
		}
		if(line.find(".equ")!=string::npos){
		getline(infile, line);
			continue;
		}
		vector<string> glb = instructionParser->getGlobalOrExtern(line); //azuriraj tabelu simbola
		if (glb.size() > 0) {
			for (unsigned i = 0; i < glb.size(); i++)
				for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
					if (ci->name == glb[i]) {
						ci->flag = 'g';
						break;
					}
					else externSymbols.push_back(glb[i]);
				}

			getline(infile, line);
			continue;
		}
		readedContent = instructionParser->getSection(line);

		if (readedContent != "") {  //ako je sekcija samo azuriraj da si u novoj sekciji
			currentSection = readedContent;
			locationCounter = 0;
			currentSectionNumber++;
			for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
						if (ci->Symbol::name == currentSection) {
						currentFlags=ci->flags;
						break;}
					}
			myfile << endl << readedContent << " section" << endl;
			if (readedContent != "bss" && currentFlags.find("p")!=string::npos) {
				RelocationRecord rec("rel. " + readedContent);
				listOfRecs.push_back(rec);
			} //dodajes novi relokacioni fajl sa imenom...bss sekcija nema relokacioni fajl...

			getline(infile, line);
			continue;
		}

		readedContent = instructionParser->getLabel(line);
		/*if (readedContent != "") {
			if(findGlb(readedContent)) //azuriraj fleg da je glb
				for (list<Symbol>::iterator ci = Worker::listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
					if (ci->name == readedContent) {
						ci->flag = 'g';
						break;
					}
				}
		}*/ // onda ako naidjem na labelu ne bi trebala da radim nista?? iii ako nema simbola u tabeli simbola onda je extern...
		//a kako ces da proveravas da li si simb definisan ako je proglasen za glb?

		if (instructionParser->getInstructionName(line) == "labelOnly") {
			getline(infile, line);
			continue;
		}

		if (currentSection == "text" || currentFlags.find("x")!=string::npos) {
			if (instructionParser->isInstruction(line)) {
				readInstructions(line); //ovde obavljas sve? i povecavas loccounter
				//locationCounter += instructionParser->getInstructionSize(instructionParser->getAddressingType(line, instructionParser->isWord(line)), instructionParser->isWord(line));
				getline(infile, line);
				continue;
			}
			else {
				smatch s;
				if (regex_search(line, s, instructionParser->tokenParsers->at(DIRECTIVE))) {
					handleDirectiveSecondPass(line);
					getline(infile, line);
					continue;
				}
			}
			throw runtime_error("Unapropriate action in text section.");
		}
		else if (currentSection == "data" || currentFlags.find("w")!=string::npos) {
			smatch s;
			if (regex_search(line, s, instructionParser->tokenParsers->at(DIRECTIVE))) {
				handleDirectiveSecondPass(line);
				getline(infile, line);
				continue;
			}
			else throw runtime_error("Unapropriate action in data section.");
		}
		else if (currentSection == "bss") {
			smatch s;
			if (regex_search(line, s, instructionParser->tokenParsers->at(DIRECTIVE))) {
				handleDirectiveSecondPass(line);
				getline(infile, line);
				continue;
			}
			else throw runtime_error("Unapropriate action in bss section.");
		}
		else //ovde razresi pitanje .section nesto,bla,bla
		{
		}
	}
	//writeRecToFile();
	myfile << endl<<"-----------------#Sadrzaji sekcija--------------------" << endl;
	for (std::list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ci++) {
		ci->writeToDocument(myfile);
	}

	myfile << endl;
	for (std::list<RelocationRecord>::iterator ci = listOfRecs.begin(); ci != listOfRecs.end(); ci++) {
		ci->writeToDocument(myfile);
	}
	myfile<<std::flush;
}


void Worker::handleDirectiveSecondPass(string line) {
	regex token("\\.(byte|word|align|skip)");
	smatch s;
	string currToken = "";
	if (regex_search(line, s, token)) {
		currToken = s[1];
	}

	if (currentSection == "bss" && currToken != "align" && currToken != "skip")
		throw runtime_error("Directive .word, .byte cant be defined in .bss section.");

	regex dec("^([0-9]+)$");
	regex hexa("^(0x[0-9abcdef]+)$");


	if (currToken == "align") {//Isto kao u prvom prolazu podesavamo vrednost LC.
		int oldCounter = locationCounter;//Msm da .align moze da se javi u bilo kojoj sekciji.
		int pom = line.find_first_of(".");
		pom += 7;
		string align = line.substr(pom, line.length());

		int i = 0;
		if (regex_match(align, s, hexa))
			i = std::stoi(align, nullptr, 16); //SAMO ZA DECIMALNO PORAVNANJE
		else if (regex_match(align, s, dec))
			i = std::stoi(align, nullptr, 10);
		else throw runtime_error("Wrong format for align directive");
		if (!(locationCounter % i == 0)) {
			locationCounter = locationCounter / i * i + i;
		}
		for (int j = oldCounter; j < locationCounter; j++) {
			myfile << std::hex << j << "  " << std::hex << (short)0 << endl; //ispisujem lokaciju i heksa kod
		}
		return;
	}

	if (currToken == "skip") {//Isto kao u prvom prolazu podesavamo vrednost LC.
		int oldCounter = locationCounter;//Msm da .skip moze da se javi u bilo kojoj sekciji.
		int pom = line.find_first_of(".");
		pom += 6;
		string align = line.substr(pom, line.length());

		int i = 0;
		if (regex_match(align, s, hexa))
			i = std::stoi(align, nullptr, 16); //SAMO ZA DECIMALNO PORAVNANJE
		else if (regex_match(align, s, dec))
			i = std::stoi(align, nullptr, 10);
		else throw runtime_error("Wrong format for skip directive");

		for (int j = 0; j < i; j++) {
			myfile << std::hex << j + locationCounter << "  " << std::hex << (short)0 << endl; //ispisujem lokaciju i heksa kod
		}
		locationCounter += i;
		return;
	}

	if (currToken == "word") {

		int pom = line.find_first_of(".");
		pom += 5;
		line = line.substr(pom, line.length());

		vector<string> seglist;
		string segment;
		std::stringstream ss(line);

		while (std::getline(ss, segment, ',')) {
			seglist.push_back(segment);
		}

		for (unsigned i = 0; i < seglist.size(); i++) {
			int pom = seglist[i].find_first_not_of(" ");
			seglist[i] = seglist[i].substr(pom, seglist[i].length()); //obrisi blanko

			bool find = false;
			int sectionNumber;


			if (regex_match(seglist[i], s, dec)) {
				myfile << std::hex << locationCounter << "  " << std::hex << reverseBytes16((short int)stoi(seglist[i], nullptr, 10)) << endl;
				locationCounter += 2;
				continue;
			}

			if (regex_match(seglist[i], s, hexa)) {
				myfile << std::hex << locationCounter << "  " << std::hex << reverseBytes16((short int)stoi(seglist[i], nullptr, 16)) << endl;
				locationCounter += 2;
				continue;
			}
			else {
				unordered_map<std::string, int>::iterator got = equList->find(seglist[i]);
				if (got != equList->end()) { //ako nadjes u equ samo ubaci vrednost
					myfile << std::hex << locationCounter << "  " << std::hex << reverseBytes16((short int)got->second) << endl;
					locationCounter += 2;
					continue; //odnosi se na for iznad
				}
				int embedded=relocation(seglist[i], false, false, locationCounter);
				find = true;
				myfile << std::hex << locationCounter << "  " << std::hex << (reverseBytes16(embedded)&0xffff) << endl;
				locationCounter += 2;
				continue;
			}
			throw runtime_error("Wrong format for word directive ");
		}

		return;
	}

	if (currToken == "byte") { //moze samo imm?

		int pom = line.find_first_of(".");
		pom += 5;
		line = line.substr(pom, line.length());

		vector<string> seglist;
		string segment;
		std::stringstream ss(line);

		while (std::getline(ss, segment, ',')) {
			seglist.push_back(segment);
		}
		for (unsigned i = 0; i < seglist.size(); i++) {
			int pom = seglist[i].find_first_not_of(" ");
			seglist[i] = seglist[i].substr(pom, seglist[i].length()); //obrisi blanko
			if (!(regex_match(seglist[i], s, dec) && !regex_match(seglist[i], s, hexa)) && !(stoi(seglist[i]) < 128))
				throw runtime_error("Use byte directive to present 1 byte(decimal or hexa number)");

			myfile << std::hex << locationCounter << "  " << std::hex << (((short)stoi(seglist[i])) & 255) << endl;
			locationCounter++;
		}
		return;
	}
}

void Worker::readInstructions(string line) {
	string instruction = instructionParser->getInstructionName(line);
	bool isWord = instructionParser->isWord(line);
	vector<string> tokens = instructionParser->tokenizeInstruction(line);
	vector<string> addrType = instructionParser->getAddressingType(line, isWord);

	int opcode = 0;
	unordered_map<string, int>::iterator got = instructionParser->instructionHash->find(instruction); //string int
	if (got != instructionParser->instructionHash->end()) {
		opcode = got->second;

		if (isWord)
			opcode = (opcode << 3) | (1 << 2);
		else opcode = (opcode << 3);
		myfile << std::hex << locationCounter << "  " << std::hex << (((short)opcode) & 255) << " "; //dodaj hex

	}
	else throw runtime_error("Wrong instruction name");

	int numofOperands = instructionParser->numberOfOperands(line);
	pcRelative = instructionParser->isRelativeInstruction(instruction);
	relative = false; //ako naidjes na $


	switch (numofOperands) {
	case 0: {
		if (addrType[0] != "zero" || tokens.size() > 0) throw runtime_error("Wrong number of operands");
		break; }

	case 1: {
		if (addrType.size() > 1) throw runtime_error("Wrong number of operands");
		if (instruction != "push" && addrType[0] == "imm") throw runtime_error("First operand can't be immediate");
		operate(addrType, tokens, 1, isWord, line);
		break;
	}

	case 2: {	if (addrType.size() > 2) throw runtime_error("Wrong number of operands");
		if (addrType[0] == "imm") throw runtime_error("First operand can't be immediate");
		operate(addrType, tokens, 2, isWord, line);
		break; }

	case -1: {throw runtime_error("Wrong number of operands"); }
	}
	locationCounter += instructionParser->getInstructionSize(addrType, isWord);
	myfile << endl;
}

int Worker::relocation(string label, bool relative, bool pcRelative, int offset) {
	int embedded = 0;
	string relocationType = "";
	for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
		if (ci->name == label) {

			int embedded = 0;
			if (ci->flag == 'g') {
				if (relative) {
					relocationType = "rel";
					embedded = -2;
				}
				else {
					relocationType = "aps";
					embedded = 0;
				}
				if (pcRelative) {
					relocationType = "rel";
				}
				int value = ci->id;
				RelocationRecord rec = RelocationRecord(relocationType, offset, value);
				listOfRecs.push_back(rec);
				return embedded;
			}
			else if (ci->flag == 'l') {
				int value = 0;
				if (relative) {
					relocationType = "rel";
					value = ci->Symbol::sectionNumber;
					embedded = ci->Symbol::offset - 2; //proveri jel -2???
				}
				else {
					relocationType = "aps";
					value = ci->Symbol::sectionNumber;
					embedded = ci->Symbol::offset;
				}

				if (pcRelative) {
					relocationType = "rel";  //poseban slucaj za pcRelativno adresiranje, jeste rel, ali instr nije rel..
				}
				RelocationRecord rec = RelocationRecord(relocationType, offset, value);
				listOfRecs.push_back(rec);
				return embedded;

			}


		}
	}//jos da se proveri da li je simbol eksterni

	for (unsigned i = 0; i < externSymbols.size(); i++) {
		if (externSymbols[i] == label) {
			if (relative) {
				relocationType = "rel";
				embedded = -2;
			}
			else {
				relocationType = "aps";
				embedded = 0;
			}
			if (pcRelative) {
				relocationType = "rel";
			}
			int value = -1;
			listOfSymbols.push_back(Symbol(label, "SYMBOL", value & 0xffff, value & 0xffff, 0, 'g'));
			RelocationRecord rec = RelocationRecord(relocationType, offset, value & 0xffff);
			listOfRecs.push_back(rec);
			return embedded;
		}
	}
	throw runtime_error("There is no label named as" + label);


}

void Worker::operate(vector<string> addrType, vector<string> tokens, int num, bool isWord, string line) {
	int addrcode = 0;
	int pom = 0;
	int iter = 0;
	int blabla = 0;
	smatch s;
	regex dec("([0-9]+)");
	regex hexa("(0x[0-9abcdef]+)");
	regex label("[a-zA-Z_][a-zA-Z0-9]*");
	regex reg("(r[0-7f])(l|h)?");
	unordered_map<string, int>::iterator got;

	for (iter; iter < num; iter++) {
		//cout<<addrType[iter]<<endl;
		got = instructionParser->addressHash->find(addrType[iter]); //string int
		addrcode = got->second;  //kod adresiranja, a sad treba da proveris operande
		if (addrType[iter] == "regDir" || addrType[iter] == "regIndir") {

			//cout<<tokens[pom]<<endl;
			bool low = false;
			if (regex_search(tokens[pom], s, reg)) {
				tokens[pom] = s[1];
				if (s[2] == "l") low = true;
				if (s[2] == "h") low = false;
			}
			//cout<<tokens[pom]<<endl;
			got = instructionParser->registerHash->find(tokens[pom]);  //string int
			if(got==instructionParser->registerHash->end())throw runtime_error("opet 580");
			blabla = got->second;
			addrcode = (addrcode << 5) | (blabla << 1);
			if (!low and !isWord)addrcode = addrcode | 1;
			myfile << "  " << std::hex << (((short)addrcode) & 255);

			pom++; //samo reg
			continue;
		}
		else

			if (addrType[iter] == "regIndir8Pom") {
				//cout<<tokens[pom]<<endl;

				if (regex_search(tokens[pom], s, reg)) {
					tokens[pom] = s[1];
				}
				//cout<<tokens[pom]<<endl;
				got = instructionParser->registerHash->find(tokens[pom]);  //string int
				if(got==instructionParser->registerHash->end())throw runtime_error("opet 600");
				blabla = got->second;
				//cout<<blabla<<endl;
				addrcode = (addrcode << 5) | (blabla << 1);
				myfile << "  " << std::hex << (((short)addrcode) & 255); //jos operand, posto je 1 byte ne moze da bude labela vec mora da bude broj


				if (regex_search(tokens[pom + 1], s, dec)) { //jos samo proveri da li su brojevi zaista 8bita

					myfile <<" "<< std::hex << (((short)stoi(tokens[pom + 1])) & 255); //dodaj hex
					pom += 2;
				}
				else
					if (regex_search(tokens[pom + 1], s, hexa)) {
						myfile <<" "<< std::hex << (((short)stoi(tokens[pom+1])) & 255); //dodaj hex
						pom += 2;
					}
					else throw runtime_error("Only decimal or hexadecimal number for regIndir8Pom");
				continue;
			}
			else

				if (addrType[iter] == "regIndir16Pom") {  //optimizuj samo za 16pom
					//relative = false; vec je u fju iznad
					//cout<<tokens[pom]<<endl;
					if (regex_search(tokens[pom], s, reg)) {
					tokens[pom] = s[1];
					}
					//cout<<tokens[pom]<<endl;
					if (tokens[pom] == "r7")relative = true;
					

					got = instructionParser->registerHash->find(tokens[pom]);  //string int
					if(got==instructionParser->registerHash->end())throw runtime_error(" linija 627"+tokens[pom]);
					blabla = got->second;
					addrcode = (addrcode << 5) | (blabla << 1);


					myfile << "  " << std::hex << (((short)addrcode) & 255);


					string lab = "";
					if (regex_search(tokens[pom + 1], s, dec)) {

						myfile << " " << std::hex << reverseBytes16((short int)stoi(s[0])); //proveri za heksa i tako to //dodaj hex
						pom += 2;
						continue;
					}
					else
						if (regex_search(tokens[pom + 1], s, hexa)) {
							myfile << " " << std::hex << reverseBytes16((short int)stoi(s[0], nullptr, 16)); //proveri da li ti stoi za heksa vraca //dodaj hex
							pom += 2;
							continue;
						}
						else {
							if (/*tokens.size() == (pom + 2) &&*/ regex_search(tokens[pom + 1], s, label)) { //RAZRESI KOMENTAR
								lab = s[0];
							} //proveri jel s[1] ili s[0].. ii jos jedan note nisam menja nigde ins kaunter..
							/*else if (tokens.size() == (pom + 1) && regex_match(tokens[pom + 1], s, label)) {
								lab = s[1];
							}*/ //znaci bilo da je reg ili imm sada imam labelu
							//treba da proverim da li se ova labela nalazi u equ listi, tabeli simbola ili tabeli glb sim..
							int val = 0;

							got = equList->find(lab);
							if (got != equList->end()) { //pretraga equliste
								myfile << " " << std::hex << reverseBytes16((short int)got->second); //dodaj hex
								pom += 2;
								continue;
							}
							else {             //pretraga tabele simbola
								bool fined = false;
								for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
									if (ci->name == lab) {
										if (currentSectionNumber == ci->sectionNumber) {//ako je u istoj sekciji
											int embedded = locationCounter + instructionParser->getInstructionSize(addrType, isWord);
											embedded = ci->offset - embedded;
											embedded &= 0xffff;
											myfile << " " << std::hex << reverseBytes16(embedded);
											fined = true;
											pom += 2;
											continue; //vidi gde se vracas sa break!!!
										}
									}
								}
								if (fined == true)continue;
								int embedded = 0;
								if (iter == 0) embedded = relocation(lab, pcRelative, relative, locationCounter + 2);
								else if (isWord && (addrType[0] == "imm" || addrType[0] == "memDir" || addrType[0] == "regIndir16pom")) embedded = relocation(lab, pcRelative, relative, locationCounter + 5);
								else if (addrType[0] == "regIndir" || addrType[0] == "regDir") embedded = relocation(lab, pcRelative, relative, locationCounter + 3);
								else  embedded = relocation(lab, pcRelative, relative, locationCounter + 4);

								myfile << " " << std::hex << reverseBytes16((short int)embedded); //dodaj hex
								pom += 2;
								continue;


							}


						}
				}
				else if (addrType[iter] == "imm" || addrType[iter] == "memDir") { //znas li devojko sta ti radis...
						if(addrType[iter]=="memDir"){
							got = equList->find(tokens[pom]);
							if (got != equList->end()){
							int value=got->second;
							got=instructionParser->addressHash->find("imm");
							addrcode=got->second;
							addrcode = addrcode << 5;
							myfile << "  " << std::hex << (((short)addrcode) & 255);
							if(!isWord)
							myfile << "  " << std::hex << (((short)value) & 255);
							else myfile << " " << std::hex << (reverseBytes16((short int)value)&0xffff); 
							pom += 1;
							continue;
												
							}
						}
						addrcode = addrcode << 5;
						myfile << "  " << std::hex << (((short)addrcode) & 255);

						string lab = "";

						if (regex_search(tokens[pom], s, dec)) {

							myfile << " " << std::hex << reverseBytes16((short int)stoi(s[0])); //proveri za heksa i tako to //dodaj hex
							pom += 1;
							continue;
						}
						else
							if (regex_search(tokens[pom], s, hexa)) {
								myfile << " " << std::hex << reverseBytes16((short int)stoi(s[0])); //proveri da li ti stoi za heksa vraca //dodaj hex
								pom += 1;
								continue;
							}
							else {
								if (/*tokens.size() == (pom + 2) &&*/ regex_search(tokens[pom], s, label)) { //RAZRESI KOMENTAR
									lab = s[0];
								}
								int val = 0;

								/*got = equList->find(lab);
								if (got != equList->end()) { //pretraga equliste
									myfile << " " << std::hex << reverseBytes16((short int)got->second); //dodaj hex
									pom += 1;
									continue;
								}
								else {*/             //pretraga tabele simbola
									relative = false; //ne moze da dodje ovde sa $, jer ide u regIndir16pom 
									bool fined = false;
									for (list<Symbol>::iterator ci = listOfSymbols.begin(); ci != listOfSymbols.end(); ++ci) {
										if (ci->name == lab) {
											if (currentSectionNumber == ci->sectionNumber) {//ako je u istoj sekciji
												int embedded = locationCounter + instructionParser->getInstructionSize(addrType, isWord);
												embedded = ci->offset - embedded;
												embedded &= 0xffff;
												myfile << " " << std::hex << reverseBytes16(embedded); //dodaj hex
												pom += 1;
												fined = true;
												break; //vidi gde se vracas sa break!!!
											}
										}
									}
									if (fined == true)continue;
									int embedded = 0;
									if (iter == 0) embedded = relocation(lab, pcRelative, relative, locationCounter + 2);
									else if (isWord && (addrType[0] == "imm" || addrType[0] == "memDir" || addrType[0] == "regIndir16pom")) embedded = relocation(lab, pcRelative, relative, locationCounter + 5);
									else if (addrType[0] == "regIndir" || addrType[0] == "regDir") embedded = relocation(lab, pcRelative, relative, locationCounter + 3);
									else  embedded = relocation(lab, pcRelative, relative, locationCounter + 4);

									myfile << " " << std::hex << reverseBytes16((short int)embedded); //dodaj hex
									pom += 1;
									continue;

								}
							
					}
					else throw runtime_error("Unrecognised instruction addressing type");
	}
}
