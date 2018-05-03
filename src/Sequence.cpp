#include <iostream>
#include "Sequence.h"
#include "Pipeline.h"
#include "SimpleCommand.h"

/**
 * Destructor.
 */
Sequence::~Sequence() {
	for( Pipeline *p : pipelines )
		delete p;
}

/**
 * Executes a sequence, i.e. runs all pipelines and - depending if the ampersand
 * was used - waits for execution to be finished or not.
 */
void Sequence::execute() {

	for( Pipeline *p : pipelines ) {
		if(p->isAsync()) {
			std::cout << "Is async" << std::endl;
		} else {
			std::cout << "Is not async" << std::endl;	  
		}
		p->execute();
	}
}
