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
	if(commands.size() == 1) {
		commands[0]->execute();
		return;
	}

	int pipeIndex = 0;
	const int pipeSize = commands.size() - 1;
	std::cout << "Number of pipes: " << pipeSize << std::endl;
	
	while (pipeIndex < pipeSize) {
		int fd[2];
		pipe(fd);
		pid_t pid = fork();

		if (pid == 0) {
			// does the ls -li | ... part
			dup2( fd[1], 1 ); // connect 1 to pipe
			close(fd[0]);
			close(fd[1]);
			commands[pipeIndex]->execute();
			exit(1);
		} else if (0 < pid) {
			// does the ... | more part
			// Look if more pipes needed?
			pid = fork();
			if(pid == 0) { 
				dup2( fd[0], 0); // connect 0 to pipe
				close(fd[1]);
				close(fd[0]);
				commands[pipeIndex + 1]->execute();
				exit(1);
			}
			else {
				int status;
				close(fd[0]);
				close(fd[1]);
				waitpid(pid, &status, 0);
				pipeIndex++;
			}
		} else {
			std::cerr << "Pipe failed" << std::endl;
		}
	}
}
