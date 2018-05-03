#include <iostream>
#include "Sequence.h"
#include "Pipeline.h"
#include "SimpleCommand.h"
#include <unistd.h>
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
			pid_t pid = fork();
			if(pid == 0) { // Child process
				std::cout << "Runs background process with id: " << getpid() << std::endl;
				p->execute();
				exit(EXIT_FAILURE);
			} else if (pid < 0) {
				std::cerr << "Command failed, can't create child process" << std::endl;
			}
		} else {
			p->execute();
		}
	}
}
