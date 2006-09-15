

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
		size_t dataSize;
		FILE *output;

		or_get_extract_thumbnail(filename, 
								 OR_THUMB_SIZE_SMALL, &thumbnail);

		thumbnailFormat = or_thumbnail_format(thumbnail);
		thumbnailSize = or_thumbnail_size(thumbnail);
		dataSize = or_thumbnail_data_size(thumbnail);

		switch (thumbnailFormat) {
		case OR_DATA_TYPE_JPEG:
			printf("Thumbnail in JPEG format, thumb size is %d\n", thumbnailSize);
			break;
		case OR_DATA_TYPE_PIXMAP_8RGB:
			printf("Thumbnail in 8RGB format, thumb size is %d\n", thumbnailSize);
			break;
		default:
			printf("Thumbnail in UNKNOWN format, thumb size is %d\n", thumbnailSize);
			break;
		}
		output = fopen("thumb.jpg", "wb");
		thumbnailData = or_thumbnail_data(thumbnail);
		fwrite(thumbnailData, dataSize, 1, output);
		fclose(output);
		printf("output %d bytes\n", dataSize);
		or_thumbnail_release(thumbnail);
	}
	else {
		printf("No input file name\n");
	}

	return 0;
}
