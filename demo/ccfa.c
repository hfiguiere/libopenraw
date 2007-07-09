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

	printf("data size = %d\n", or_rawdata_data_size(rawdata));
	printf("data type = %d\n", or_rawdata_format(rawdata));

	if(or_rawdata_format(rawdata) == OR_DATA_TYPE_CFA) {
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
	fwrite(or_rawdata_data(rawdata), 1, or_rawdata_data_size(rawdata), f);
	fclose(f);

	or_rawdata_release(rawdata);
	
	return 0;
}

