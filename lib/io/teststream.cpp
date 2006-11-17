
#include <iostream>
#include <cstdlib>

#include "stream.h"
#include "../iofile.h"
#include "streamclone.h"

using namespace OpenRaw;

int main (int argc, char ** argv)
{
	IO::File *file = new IO::File("testfile.tmp");
	char buf1[128];
	int ret = file->open();
	if (ret != 0) {
		std::cerr << "failed: " __FILE__ ": "  << __LINE__ << std::endl;
		exit(1);
	}

	size_t r = file->read(buf1, 6);
	if (r != 6) {
		std::cerr << "failed: "  __FILE__ ": " << __LINE__ << std::endl;
		exit(1);
	}
	
	IO::StreamClone * clone = new IO::StreamClone(file, 2);

	ret = clone->open();
	if (ret != 0) {
		std::cerr << "failed: " __FILE__ ": "  << __LINE__ << std::endl;
		exit(1);
	}
	char buf2[128];
	r = file->read(buf2, 4);
	if (r != 4) {
		std::cerr << "failed: "  __FILE__ ": " << __LINE__ << std::endl;
		exit(1);
	}

	if (strncmp(buf1 + 2, buf2, 4) != 0) {
		std::cerr << "failed: "  __FILE__ ": " << __LINE__ << std::endl;
		exit(1);
	}
	clone->close();

	file->close();
}
