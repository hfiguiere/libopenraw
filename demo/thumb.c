

#include <stdio.h>

#include <libopenraw/libopenraw.h>



int
main(int argc, char **argv)
{
	char thumbFile[256];
	char *filename = argv[1];
	ORThumbnailRef thumbnail = NULL;

	if(filename && *filename)
	{
		void *thumbnailData;
		or_data_type thumbnailFormat;
		int thumbnailSize;
		FILE *output;

		or_get_extract_thumbnail(filename, 
								 OR_THUMB_SIZE_SMALL, &thumbnail);

		thumbnailFormat = or_thumbnail_format(thumbnail);
		thumbnailSize = or_thumbnail_size(thumbnail);
		
		switch (thumbnailFormat) {
		case OR_DATA_TYPE_JPEG:
			printf("Thumbnail in JPEG format, size is %d\n", thumbnailSize);
			break;
		case OR_DATA_TYPE_PIXMAP:
			printf("Thumbnail in PIXMAP format, size is %d\n", thumbnailSize);
			break;
		default:
			printf("Thumbnail in UNKNOWN format, size is %d\n", thumbnailSize);
			break;
		}
		sprintf(thumbFile, "%s.thumb", filename);
		output = fopen(thumbFile, "wb");
		fwrite(thumbnailData, thumbnailSize, 1, output);
		fclose(output);
		or_thumbnail_release(thumbnail);
	}
	else {
		printf("No input file name\n");
	}

	return 0;
}
