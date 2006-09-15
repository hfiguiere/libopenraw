

#include <iostream>
#include <libopenraw/libopenraw.h>

// FIXME
#include "thumbnail.h"
#include "debug.h"

using OpenRaw::Thumbnail;

int
main(int argc, char** argv)
{

	if (argc < 2) {
		std::cerr << "missing parameter" << std::endl;
		return 1;
	}

	Debug::Trace::setDebugLevel(Debug::DEBUG2);

	Thumbnail * thumb =
		Thumbnail::getAndExtractThumbnail(argv[1],
													 OR_THUMB_SIZE_SMALL);
	std::cerr << "thumb data size =" << thumb->size() << std::endl;
	std::cerr << "thumb data type =" << thumb->dataType() << std::endl;

	FILE * f = fopen("thumb.jpg", "wb");
	fwrite(thumb->data(), 1, thumb->size(), f);
	fclose(f);

	delete thumb;

	thumb =
		Thumbnail::getAndExtractThumbnail(argv[1],
													 OR_THUMB_SIZE_LARGE);
	std::cerr << "thumb data size =" << thumb->size() << std::endl;
	std::cerr << "thumb data type =" << thumb->dataType() << std::endl;

	f = fopen("thumbl.jpg", "wb");
	fwrite(thumb->data(), 1, thumb->size(), f);
	fclose(f);

	delete thumb;

	return 0;
}

