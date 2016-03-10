/*
 * libopenraw - thumbcpp
 *
 * Copyright (C) 2006-2016 Hubert Figuiere
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

#include "thumbnail.hpp"
#include "rawfile.hpp"

#include <boost/scoped_ptr.hpp>

using OpenRaw::Thumbnail;

void writeThumbnail(const std::unique_ptr<Thumbnail> & thumb,
                    const char* basename)
{
    FILE * f;
    size_t s;
    auto thumbnailFormat = thumb->dataType();
    std::cerr << "thumb data size =" << thumb->size() << std::endl;
    std::cerr << "thumb data type =" << thumbnailFormat << std::endl;

    std::string filename = basename;

    switch (thumbnailFormat) {
    case OR_DATA_TYPE_JPEG:
        filename += ".jpg";
        break;
    case OR_DATA_TYPE_PIXMAP_8RGB:
        filename += ".ppm";
        break;
    default:
        std::cerr << "invalid format" << std::endl;
        return;
    }

    f = fopen(filename.c_str(), "wb");
    if(thumbnailFormat == OR_DATA_TYPE_PIXMAP_8RGB) {
        fprintf(f, "P6\n");
        fprintf(f, "%u\n%u\n", thumb->width(), thumb->height());
        fprintf(f, "%d\n", 255);
    }

    s = fwrite(thumb->data(), 1, thumb->size(), f);
    if(s != thumb->size()) {
        std::cerr << "short write of " << s << " bytes\n";
    }
    fclose(f);
}


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

    {
        std::unique_ptr<OpenRaw::RawFile> raw_file(
            OpenRaw::RawFile::newRawFile(argv[1]));
        if(!raw_file)
        {
            std::cout << "Unable to open raw file.\n";
            return 1;
        }
        auto list = raw_file->listThumbnailSizes();
        for(auto elem : list)
        {
            std::cout << "found " << elem << " pixels\n";
        }
    }

    {
        std::unique_ptr<Thumbnail> thumb(
            Thumbnail::getAndExtractThumbnail(argv[1],
                                              160, err));
        if (thumb != NULL) {
            writeThumbnail(thumb, "thumb");
        }
        else {
            std::cerr << "error = " << err << std::endl;
        }
    }

    {
        std::unique_ptr<Thumbnail> thumb(
            Thumbnail::getAndExtractThumbnail(argv[1],
                                              640, err));
        if (thumb != NULL) {
            writeThumbnail(thumb, "thumbl");
        }
        else {
            std::cerr << "error = " << err << std::endl;
        }
    }

    {
        std::unique_ptr<Thumbnail> thumb(
            Thumbnail::getAndExtractThumbnail(argv[1],
                                              2048, err));
        if (thumb != NULL) {
            writeThumbnail(thumb, "preview");
        }
        else {
            std::cerr << "error = " << err << std::endl;
        }
    }

    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
