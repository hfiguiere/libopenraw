/*
 * libopenraw - or_tiff.c
 *
 * Copyright (C) 2005 Hubert Figuiere
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <tiffio.h>

#include "or_tiff.h"


or_error 
tiff_get_thumbnail(RawFileRef raw_file, ORThumbnailRef thumbnail)
{
	or_error err = OR_ERROR_NONE;

	TIFF *tif = raw_tiff_open(raw_file);
	if (tif == NULL) {
		/*err = ;*/
	}
	else {
		if(TIFFReadDirectory(tif)) {
			tsize_t read_size;
			int compression;

			compression = TIFFGetField(tif, TIFFTAG_COMPRESSION);
			if(compression == COMPRESSION_OJPEG) {
				tsize_t size = TIFFStripSize(tif);

				thumbnail->data = (char*)malloc(size);
				read_size = TIFFReadRawStrip(tif, 0, thumbnail->data, size);
				thumbnail->data_size = read_size;
				thumbnail->data_type = OR_DATA_TYPE_JPEG;
				thumbnail->thumb_size = OR_THUMB_SIZE_SMALL;
				thumbnail->x = TIFFGetField(tif, TIFFTAG_IMAGEWIDTH);
				thumbnail->y = TIFFGetField(tif, TIFFTAG_IMAGELENGTH);
			}
			else {
				/* FIXME */
			}
		}

		TIFFClose(tif);
	}

	return err;
}
