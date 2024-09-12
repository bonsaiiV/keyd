#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void xwrite(int fd, const void *buf, size_t sz)
{
	char *str = (char*) buf;
	size_t nwr = 0;
	ssize_t n;

	while(sz != nwr) {
		n = write(fd, str+nwr, sz-nwr);
		if (n < 0) {
			perror("write");
			exit(-1);
		}
		nwr += n;
	}
}

void xread(int fd, void *buf, size_t sz)
{
	char *str = (char*) buf;
	size_t nrd = 0;
	ssize_t n;

	while(sz != nrd) {
		n = read(fd, str+nrd, sz-nrd);
		if (n < 0) {
			perror("read");
			exit(-1);
		}
		nrd += n;
	}
}

