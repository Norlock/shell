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
	int currentIndex = 0;

	if(commands.size() == 1) {
		commands[0]->execute();
		return;
	}

	while (hasMorePipes(currentIndex)) {

		pid_t pid;
		int fd[2];

		pipe(fd);
		pid = fork();

		if (pid == 0) {
			// does the ls -li | ... part
			dup2( fd[1], 1 ); // connect 0 to pipe
			close(fd[0]);
			close(fd[1]);
			commands[currentIndex]->execute();
			exit(1);
		} else if (0 < pid) {
			// does the ... | more part
			pid = fork();
			if(pid == 0) { 
				dup2( fd[0], 0); // connect 1 to pipe
				close(fd[1]);
				close(fd[0]);
				commands[currentIndex + 1]->execute();
				exit(1);
				if(hasMorePipes(currentIndex)) continue;
			}
			else {
				int status;
				close(fd[0]);
				close(fd[1]);
				waitpid(pid, &status, 0);
				currentIndex++; 
			}
		} else {
			std::cerr << "Pipe failed" << std::endl;
		}
	}
}

bool Pipeline::hasMorePipes(const int currentIndex) {
	return currentIndex < commands.size() - 1; // currentIndex still lower than last index?
}
