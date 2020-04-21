#include "InstructionParser.h"
#include <iostream>
#include "Worker.h"

using namespace std;
int main(int argc, char** argv) {
	try{
		Worker *worker=new Worker(argv[1], argv[2]);
		worker->firstPass();
		worker->secondPass();
	}
	catch (exception e) { cout << e.what(); }
	

	return 0;
}
