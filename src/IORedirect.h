#ifndef SHELL_IOREDIRECT_H
#define SHELL_IOREDIRECT_H

#include <string>

class IORedirect
{
	public:
		enum Type { OUTPUT, APPEND, INPUT };
		enum Fd { STDIN = 0, STDOUT = 1, STDERR = 2, STDFILE = 3 };

	private:
		int oldFileDesc;      //< The file descriptor that is subject of redirection.
		Type type;            //< Type of redirection.
		std::string newFile;  //< Destination for the redirect. This can be a filename, or - if
		                      //  it starts with an ampersand ('&') another file descriptor. E.g.
		                      //  if this is '&1', output will be written to stdout.

	public:
		IORedirect( int oldFd, Type t, std::string const &nf )
				: oldFileDesc(oldFd), type(t), newFile(nf)
		{}

		int getOldFileDescriptor() const { return oldFileDesc; }
		Type getType() const { return type; }
		std::string const &getNewFile() const { return newFile; }
};

#endif //SHELL_IOREDIRECT_H
