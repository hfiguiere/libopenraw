/*
 * libopenraw - thumbcpp
 *
 * Copyright (C) 2006, 2009 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <stdio.h>

#include <iostream>
#include <libopenraw/libopenraw.h>
#include <libopenraw/debug.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawfile.h>

#include <boost/scoped_ptr.hpp>

using OpenRaw::Thumbnail;
using boost::scoped_ptr;

int
main(int argc, char** argv)
{
	::or_error err = OR_ERROR_NONE;

	if (argc < 2) {
		std::cerr << "missing parameter" << std::endl;
		return 1;
	}

	OpenRaw::init();
	or_debug_set_level(DEBUG2);
	FILE * f;

	{
		scoped_ptr<OpenRaw::RawFile> raw_file(OpenRaw::RawFile::newRawFile(argv[1]));
		if(!raw_file)
		{
			std::cout << "Unable to open raw file.\n";
			return 1;
		}
		std::vector<uint32_t> list = raw_file->listThumbnailSizes();
	
		for(std::vector<uint32_t>::iterator i = list.begin();
				i != list.end(); ++i)
		{
			std::cout << "found " << *i << " pixels\n";
		}
	}

	{
		scoped_ptr<Thumbnail> thumb(Thumbnail::getAndExtractThumbnail(argv[1],
																	  160, err));
		if (thumb != NULL) {
			size_t s;
			std::cerr << "thumb data size =" << thumb->size() << std::endl;
			std::cerr << "thumb data type =" << thumb->dataType() << std::endl;
			
			f = fopen("thumb.jpg", "wb");
			s = fwrite(thumb->data(), 1, thumb->size(), f);
			if(s != thumb->size()) {
				std::cerr << "short write of " << s << " bytes\n";
			}
			fclose(f);
		}
		else {
			std::cerr << "error = " << err << std::endl;
		}
	}

	{
		scoped_ptr<Thumbnail> thumb(Thumbnail::getAndExtractThumbnail(argv[1],
																	  640, err));
		
		if (thumb != NULL) {
			size_t s;
			std::cerr << "thumb data size =" << thumb->size() << std::endl;
			std::cerr << "thumb data type =" << thumb->dataType() << std::endl;
			
			f = fopen("thumbl.jpg", "wb");
			s = fwrite(thumb->data(), 1, thumb->size(), f);
			if(s != thumb->size()) {
				std::cerr << "short write of " << s << " bytes\n";
			}
			fclose(f);
		}
		else {
			std::cerr << "error = " << err << std::endl;
		}
	}

	{
		scoped_ptr<Thumbnail> thumb(Thumbnail::getAndExtractThumbnail(argv[1],
																	  2048, err));
		if (thumb != NULL) {
			size_t s;
			std::cerr << "preview data size =" << thumb->size() << std::endl;
			std::cerr << "preview data type =" << thumb->dataType() << std::endl;
			
			f = fopen("preview.jpg", "wb");
			s = fwrite(thumb->data(), 1, thumb->size(), f);
			if(s != thumb->size()) {
				std::cerr << "short write of " << s << " bytes\n";
			}
			fclose(f);
		}
		else {
			std::cerr << "error = " << err << std::endl;
		}
	}

	return 0;
}

