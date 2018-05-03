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

	int status, commandIndex = 0;
	const int pipeSize = commands.size() - 1;
	int pipeFdsOld[2], pipeFdsNew[2];

	while (commands[commandIndex]) {
		bool hasNextCommand = commandIndex < pipeSize;
		bool hasPreviousCommand = 0 < commandIndex;

		if(hasNextCommand) {
			if(pipe(pipeFdsNew) < 0) {
				perror("Pipe error 4");
				exit(EXIT_FAILURE);
			}
		}

		pid_t pid = fork();
		if (pid == 0) {

			if(hasPreviousCommand) {
				if(dup2(pipeFdsOld[0], STDIN_FILENO) < 0) {
					perror("Pipe error 3");
					exit(EXIT_FAILURE);
				}
				close(pipeFdsOld[0]);
				close(pipeFdsOld[1]);
			}

			if(hasNextCommand) {
				close(pipeFdsNew[0]);
				if(dup2(pipeFdsNew[1], STDOUT_FILENO) < 0) {
					perror("Pipe error 2");
					exit(EXIT_FAILURE);
				}
				close(pipeFdsNew[1]);
			}

			commands[commandIndex]->execute();
			exit(EXIT_FAILURE);
		} else if (pid < 0) {
			perror("Pipe error 1");
			exit(EXIT_FAILURE);
		} else {
			if(hasPreviousCommand) {
				close(pipeFdsOld[0]);
				close(pipeFdsOld[1]);
			}

			if(hasNextCommand) {
				pipeFdsOld[0] = pipeFdsNew[0];
				pipeFdsOld[1] = pipeFdsNew[1];
			}
		}

		commandIndex++;
	}

	// Parent closes all of its copies
	close(pipeFdsOld[0]);
	close(pipeFdsOld[1]);

	for(int i = 0; i < pipeSize + 1; i++)
		wait(&status);
}
