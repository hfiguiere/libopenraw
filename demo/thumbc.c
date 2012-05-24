/*
 * libopenraw - thumbc.c
 *
 * Copyright (C) 2006,2008 Hubert Figuiere
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
#include <unistd.h>
#include <stdlib.h>

#include <libopenraw/libopenraw.h>

int
main(int argc, char **argv)
{
    char *filename;
    int thumb_size = 160;
    int opt;
    ORThumbnailRef thumbnail = NULL;

    void *thumbnailData;
    or_data_type thumbnailFormat;
    size_t dataSize;
    size_t writtenSize;
    FILE *output;
    uint32_t x, y;
    or_error err;
    const char* outfname = "thumb.raw";
    
    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch(opt) {
        case 's':
            thumb_size = atoi(optarg);
            break;
        default:
            break;
        }
    }
    
    if(optind >= argc) {
        fprintf(stderr, "Missing filename\n");
        return 1;
    }
    filename = argv[optind];
    (void)argc;
    
    or_debug_set_level(DEBUG2);
    
    if(!filename || !*filename)
    {
        printf("No input file name\n");
        return 1;
    }
    
    err = or_get_extract_thumbnail(filename, 
                                   thumb_size, &thumbnail);
    
    if (err != OR_ERROR_NONE) {
        printf("error %d\n", err);
        return 1;
    }

    thumbnailFormat = or_thumbnail_format(thumbnail);
    dataSize = or_thumbnail_data_size(thumbnail);
    or_thumbnail_dimensions(thumbnail, &x, &y);
    
    switch (thumbnailFormat) {
    case OR_DATA_TYPE_JPEG:
        printf("Thumbnail in JPEG format, thumb size is %u, %u\n", x, y);
        outfname = "thumb.jpg";
        break;
    case OR_DATA_TYPE_PIXMAP_8RGB:
        printf("Thumbnail in 8RGB format, thumb size is %u, %u\n", x, y);
        outfname = "thumb.ppm";
        break;
    default:
        printf("Thumbnail in UNKNOWN format, thumb size is %u, %u\n", x, y);
        break;
    }
    output = fopen(outfname, "wb");
    thumbnailData = or_thumbnail_data(thumbnail);
    if(thumbnailFormat == OR_DATA_TYPE_PIXMAP_8RGB) {
        fprintf(output, "P6\n");
        fprintf(output, "%u\n%u\n", x, y);
        fprintf(output, "%d\n", 255);
    }
    writtenSize = fwrite(thumbnailData, dataSize, 1, output);
    if(writtenSize != dataSize) {
        printf("short write\n");
    }
    fclose(output);
    printf("output %ld bytes in '%s'\n", dataSize, outfname);
    err = or_thumbnail_release(thumbnail);
    if (err != OR_ERROR_NONE)
    {
        printf("error release %d\n", err);
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
