

#include <iostream>
#include <libopenraw/libopenraw.h>

// FIXME
#include "thumbnail.h"
#include "debug.h"
#include "rawfile.h"

using OpenRaw::Thumbnail;

int
main(int argc, char** argv)
{

	if (argc < 2) {
		std::cerr << "missing parameter" << std::endl;
		return 1;
	}

	OpenRaw::init();
	Debug::Trace::setDebugLevel(Debug::DEBUG2);
	FILE * f;

	OpenRaw::RawFile * raw_file = OpenRaw::RawFile::newRawFile(argv[1]);
	std::vector<uint32_t> list = raw_file->listThumbnailSizes();
	
	for(std::vector<uint32_t>::iterator i = list.begin();
			i != list.end(); ++i)
	{
		std::cout << "found " << *i << " pixels\n";
	}

	delete raw_file;

	Thumbnail * thumb =
		Thumbnail::getAndExtractThumbnail(argv[1],
													 160);
	if (thumb != NULL) {
		std::cerr << "thumb data size =" << thumb->size() << std::endl;
		std::cerr << "thumb data type =" << thumb->dataType() << std::endl;
		
		f = fopen("thumb.jpg", "wb");
		fwrite(thumb->data(), 1, thumb->size(), f);
		fclose(f);
		
		delete thumb;
	}

	thumb =
		Thumbnail::getAndExtractThumbnail(argv[1],
													 640);

	if (thumb != NULL) {
		std::cerr << "thumb data size =" << thumb->size() << std::endl;
		std::cerr << "thumb data type =" << thumb->dataType() << std::endl;
		
		f = fopen("thumbl.jpg", "wb");
		fwrite(thumb->data(), 1, thumb->size(), f);
		fclose(f);
		
		delete thumb;
	}

	thumb =
		Thumbnail::getAndExtractThumbnail(argv[1],
													 2048);
	if (thumb != NULL) {
		std::cerr << "preview data size =" << thumb->size() << std::endl;
		std::cerr << "preview data type =" << thumb->dataType() << std::endl;
		
		f = fopen("preview.jpg", "wb");
		fwrite(thumb->data(), 1, thumb->size(), f);
		fclose(f);
		
		delete thumb;
	}


	return 0;
}

