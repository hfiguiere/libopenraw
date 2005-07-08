

#include "thumbnails.h"

#include <libopenraw/libopenraw.h>


ORThumbnailRef openraw_thumbnail_new(void)
{
    ORThumbnailRef obj;

    obj = (ORThumbnailRef)malloc(sizeof(struct _ORThumbnail));
    memset(obj, 0, sizeof(struct _ORThumbnail));
    return obj;
}

or_error openraw_thumbnail_release(ORThumbnailRef thumb)
{
    if (thumb == NULL) {
        return OR_ERROR_NOTAREF;
    }
    if (thumb->data) {
        free(thumb->data);
    }
    free(thumb);
    return OR_ERROR_NONE;
}

or_error openraw_get_extract_thumbnail(const char* filename,
				       or_thumb_size preferred_size,
				       ORThumbnailRef *thumbnail)
{
    or_error err = OR_ERROR_NONE;
    RawFileRef raw_file;

    if (*thumbnail == NULL) {
        thumbnail = openraw_thumbnail_new();
    }

    raw_file = raw_open(get_default_io_methods(), filename, O_RDONLY);

    cr2_get_thumbnail(raw_file, thumbnail);

    raw_close(raw_file);
    raw_file = NULL;
    
    return err;
}
