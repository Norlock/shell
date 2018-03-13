#include <iostream>
#include "SimpleCommand.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "string.h"
#include <sys/types.h> 
#include <sys/wait.h>

void SimpleCommand::execute() {

	if(command == "cd") {
		const char *argumentPath = arguments[0].c_str();
		int ret;
		ret = chdir(argumentPath);
		if(ret == -1) 
			std::cerr << "Directory does not exist" << std::endl;

		return;
	}

	const int argumentsInputSize = arguments.size(); 
	const int argumentsArraySize = argumentsInputSize + 2; // + 2 for filename and NULL termination 
	char* parmList[argumentsArraySize]; 
	parmList[0] = (char*)command.c_str();

	if(argumentsInputSize > 0) {
		for(std::vector<int>::size_type i = 0; i != argumentsInputSize; i++) {
			parmList[i+1] = strdup((char*)arguments[i].c_str());
		}
	}
	parmList[argumentsArraySize - 1] = NULL; 

	pid_t pid;
	if ((pid = fork()) == -1)
		perror("fork() error");
	else if (pid == 0) {
		execvp(command.c_str(), parmList);
		std::cerr << "Return not expected. Process failed." << std::endl;
	}

	waitpid(pid, NULL, 0); // Don't write to stdout before child process is finished.
}
