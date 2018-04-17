#include <iostream>
#include <fstream>      // std::ifstream
#include "SimpleCommand.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "string.h"
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>

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

	if(redirects.size() > 0) {
		for(std::vector<int>::size_type i = 0; i != redirects.size(); i++) {
			int redirectType = redirects[i].getType();
			std::string filePath = redirects[i].getNewFile();

			if(redirectType == IORedirect::OUTPUT) {
				std::cout << "Is output" << std::endl; 

				const int fileDescriptor = open(filePath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
				std::cout << "fileDescriptor: " << fileDescriptor << std::endl;

				if(fileDescriptor != -1) { // Does File exist?
					execRedirect(IORedirect::STDOUT, fileDescriptor, parmList);
				}
			}
			else if (redirectType == IORedirect::INPUT) {
				std::cout << "Is input" << std::endl; 

				const int fileDescriptor = open(filePath.c_str(), O_RDONLY);
				std::cout << "fileDescriptor: " << fileDescriptor << std::endl;

				if(fileDescriptor != -1) {
					execRedirect(IORedirect::STDIN, fileDescriptor, parmList);
				}
				else {
					std::cerr << "File not found" << std::endl;
				}
			}
			else if (redirectType == IORedirect::APPEND) {
				std::cout << "Is append" << std::endl; 

				const int fileDescriptor = open(filePath.c_str(), O_WRONLY | O_APPEND);
				std::cout << "fileDescriptor: " << fileDescriptor << std::endl;

				if(fileDescriptor != -1) { // Does File exist?
					execRedirect(IORedirect::STDOUT, fileDescriptor, parmList);
				}
			}
		}
	}
	else {

		if ((pid = fork()) == -1)
			perror("fork() error");
		else if (pid == 0) {
			execvp(command.c_str(), parmList);
			std::cerr << "Return not expected. Process failed." << std::endl;
		}
		waitpid(pid, NULL, 0); // Don't write to stdout before child process is finished.
	}
}

void SimpleCommand::execRedirect(const int fdFrom, const int fdTo, char* parmList[]) {
	if ((pid = fork()) == -1) {
		std::cerr << "Error fork failed" << std::endl;
		return;
	}
	else if (pid == 0) {
		const int dupReturn = dup2(fdTo, fdFrom);
		if(dupReturn == -1) {
			std::cerr << "Redirect failed, make sure you have permissions." << std::endl;
			return;
		}	
		else {
			execvp(command.c_str(), parmList);
			std::cerr << "Return not expected. Process failed." << std::endl;
		}
	}
	waitpid(pid, NULL, 0); // Don't write to stdout before child process is finished.
}
