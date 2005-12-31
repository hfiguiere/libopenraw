


#include <libopenraw/libopenraw.h>

/** real thumbnail extracted */
struct _ORThumbnail 
{
	/** raw data */
	char *data;
	/** size in bytes of raw data */
	size_t data_size;
	/** x dimension in pixels of thumbnail data */
	int x;
	/** y dimension in pixels of thumbnail data */
	int y;
	/** size type of thumbnail */
	or_thumb_size thumb_size;
};



