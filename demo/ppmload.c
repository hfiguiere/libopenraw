/*
 * libopenraw - ppmload.c
 *
 * Copyright (C) 2007, 2010 Hubert Figuiere
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

/* TODO use configure instead. */
#if defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
# define htobe16(x) OSSwapHostToBigInt16(x)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
# include <sys/endian.h>
#else
# include <endian.h>
#endif

int
main(int argc, char **argv)
{
    const char *filename;
    
    if(argc < 2) {
        return 1;
    }
    
    filename = argv[1];
    
    or_debug_set_level(DEBUG2);
    
    if(filename && *filename) {
        ORRawFileRef raw_file = or_rawfile_new(filename, OR_RAWFILE_TYPE_UNKNOWN);
	
        if(raw_file) {
            or_error err;
            ORBitmapDataRef bitmapdata = or_bitmapdata_new();
            err = or_rawfile_get_rendered_image(raw_file, bitmapdata, 0);
            if(err == OR_ERROR_NONE) {
                uint32_t x, y;
                FILE * f;
                size_t size, written_size, i;
                uint16_t* data;		
                or_data_type format = or_bitmapdata_format(bitmapdata);
                size_t componentsize = (format == OR_DATA_TYPE_PIXMAP_16RGB) ? 2 : 1;

                or_bitmapdata_dimensions(bitmapdata, &x, &y);
                printf(" --- dimensions x = %d, y = %d\n", x, y);
                f = fopen("image.ppm", "wb");
                fprintf(f, "P6\n");
                fprintf(f, "%d %d\n", x, y);
                fprintf(f, "%d\n", (componentsize == 2) ? 0xffff : 0xff);
                
                size = or_bitmapdata_data_size(bitmapdata);
                printf(" --- size = %ld\n", (long)size);
                data = (uint16_t*)or_bitmapdata_data(bitmapdata);

                if(componentsize == 2) {
                    written_size = 0;
                    for(i = 0; i < size; i+=2) {
                        uint16_t value = htobe16(data[i/2]);
                        written_size += fwrite(&value, 1, sizeof(value), f);
                    }
                }
                else {
                    written_size = fwrite(or_bitmapdata_data(bitmapdata), 1, size, f);
                }
                if(written_size != size) {
                    printf("short read\n");
                }
                fclose(f);
            }
            or_bitmapdata_release(bitmapdata);
            or_rawfile_release(raw_file);
        }
    }
    else {
        printf("No input file name\n");
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
