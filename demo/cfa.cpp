/*
 * libopenraw - cfa.cpp
 *
 * Copyright (C) 2007-2016 Hubert Figuiere
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
#include <stdlib.h>

#include <iostream>
#include <libopenraw/libopenraw.h>
#include <libopenraw/debug.h>

#include "thumbnail.hpp"
#include "rawfile.hpp"
#include "rawdata.hpp"

#include <boost/scoped_ptr.hpp>

using OpenRaw::Thumbnail;
using boost::scoped_ptr;

int
main(int argc, char** argv)
{
    bool keepCompressed = false;
    if (argc < 2) {
        std::cerr << "missing parameter" << std::endl;
        return 1;
    }
    
    int c;
    do {
        c = getopt(argc, argv, "r");
        if(c != -1) {
            if(c == 'r') {
                keepCompressed = true;
            }
        }
    } while(c != -1);
    
    OpenRaw::init();
    or_debug_set_level(DEBUG2);
    FILE * f;
    size_t written_size;
    
    scoped_ptr<OpenRaw::RawFile> raw_file(OpenRaw::RawFile::newRawFile(argv[optind]));
    
    OpenRaw::RawData rdata;
    uint32_t options = (keepCompressed ? OR_OPTIONS_DONT_DECOMPRESS : 0);
    raw_file->getRawData(rdata, options);
    
    if(keepCompressed) {
        std::cout << "keep compressed" << std::endl;
    }
    std::cout << "data size = " << rdata.size() << std::endl;
    std::cout << "data type = " << rdata.dataType() << std::endl;
    
    if(!keepCompressed && rdata.dataType() == OR_DATA_TYPE_RAW) {
        f = fopen("image.pgm", "wb");
        fprintf(f, "P5\n");
        fprintf(f, "%d %d\n", rdata.width(), rdata.height());
        fprintf(f, "%d\n", (1 << rdata.bpc()) - 1);
    }
    else {
        f = fopen("image.cfa", "wb");
    }
    // Convert data byte order to most significant byte first
    if(rdata.bpc() == 16) {
        uint8_t* buf = (uint8_t*)malloc(rdata.size());
        uint8_t* p = buf;
        uint16_t* n = reinterpret_cast<uint16_t*>(rdata.data());
        for(size_t i = 0; i < rdata.size() / 2; i++) {
            unsigned char lo = n[i] & 0xFF;
            unsigned char hi = n[i] >> 8;
            p[i * 2]   = hi;
            p[i * 2 + 1] = lo;
        }
        written_size = fwrite(buf, 1, rdata.size(), f);
        free(buf);
    }
    else {
        written_size = fwrite(rdata.data(), 1, rdata.size(), f);
    }
    if (written_size != rdata.size()) {
        printf("short write\n");
    }
    fclose(f);
    
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
