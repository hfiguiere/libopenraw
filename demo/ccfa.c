/*
 * libopenraw - ccfa.c
 *
 * Copyright (C) 2007 Hubert Figuiere
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

#include <libopenraw/libopenraw.h>
#include <libopenraw/debug.h>



int
main(int argc, char** argv)
{
    ORRawDataRef rawdata;
    or_error err;
    int c;
    uint32_t options;
    FILE *f;
    int keepCompressed = 0;
    size_t written_size;
    
    if (argc < 2) {
        fprintf(stderr, "missing parameter\n");
        return 1;
    }
    
    do {
        c = getopt(argc, argv, "r");
        if(c != -1) {
            if(c == 'r') {
                keepCompressed = 1;
            }
        }
    } while(c != -1);
    
    options = (keepCompressed ? OR_OPTIONS_DONT_DECOMPRESS : 0);
    err = or_get_extract_rawdata(argv[optind], options,
                                 &rawdata);
    if(err != OR_ERROR_NONE) {
        printf("Error extracting CFA. %d\n", err);
    }
    printf("data size = %ld\n", or_rawdata_data_size(rawdata));
    printf("data type = %d\n", or_rawdata_format(rawdata));
    
    if(!keepCompressed && or_rawdata_format(rawdata) == OR_DATA_TYPE_RAW) {
        uint32_t x, y, bpc;
        or_rawdata_dimensions(rawdata, &x, &y);
        bpc = or_rawdata_bpc(rawdata);
        f = fopen("image.pgm", "wb");
        fprintf(f, "P5\n");
        fprintf(f, "%d %d\n", x, y);
        fprintf(f, "%d\n", (1 << bpc) - 1);
    }
    else {
        f = fopen("image.cfa", "wb");
    }
    /* Convert data byte order to most significant byte first */
    if(or_rawdata_bpc(rawdata) == 16) {
        size_t size = or_rawdata_data_size(rawdata);
        uint8_t* buf = (uint8_t*)malloc(size);
        uint8_t* p = buf;
        uint16_t* n = (uint16_t*)or_rawdata_data(rawdata);
        size_t i;
        
        for(i = 0; i < size / 2; i++) {
            unsigned char lo = n[i] & 0xFF;
            unsigned char hi = n[i] >> 8;
            p[i * 2]   = hi;
            p[i * 2 + 1] = lo;
        }
        written_size = fwrite(buf, 1, size, f);
        free(buf);
        if(written_size != size) {
            printf("short read\n");
        }
    }
    else {
        size_t size = or_rawdata_data_size(rawdata);
        written_size = fwrite(or_rawdata_data(rawdata), 1, size, f);
        if(written_size != size) {
            printf("short read\n");
        }
    }
    fclose(f);
    
    or_rawdata_release(rawdata);
    
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
