#include <iostream>
#include "Pipeline.h"
#include "SimpleCommand.h"
#include <unistd.h>
#include <sys/wait.h>

/**
 * Destructor.
 */
Pipeline::~Pipeline() {
	for( SimpleCommand *cmd : commands )
		delete cmd;
}

/**
 * Executes the commands on this pipeline.
 */
void Pipeline::execute() {
	const int commandsSize = commands.size();
	int currentIndex = 0;
	bool hasMorePipes = true;

	if(commandsSize == 1) {
		commands[0]->execute();
		return;
	}
	//while (hasMorePipes) {
	// FIXME: Probably need to set up some pipe here?

	int pid;
	int p[2];
	pipe(p);

	pid = fork();
	std::cout << "pid: " << pid << std::endl;
	hasMorePipes = false;

	if (pid == 0) {
		// does the ... | more part
		std::cout << "right pipe: " << commands[currentIndex + 1]->command << std::endl;
		dup2( p[0], 0 ); // connect 0 to pipe
		close(p[1]);
		close(p[0]);
		commands[currentIndex + 1]->execute();
		exit(EXIT_SUCCESS);
	} else if (0 < pid) {
		// does the ls -li | ... part
		std::cout << "left pipe: " << commands[0]->command << std::endl;
		dup2( p[1], 1 ); // connect 1 to pipe
		close(p[0]);
		close(p[1]);
		commands[currentIndex]->execute();
	} else {
		std::cerr << "Pipe failed" << std::endl;
	}

	std::cout << "Current pid: " << getpid() << std::endl;
	std::cout << "Komt hier ook" << std::endl;
}
