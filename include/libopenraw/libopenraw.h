




#ifndef __LIBOPENRAW_H__
#define __LIBOPENRAW_H__

#include <libopenraw/io.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _ORThumbnail *ORThumbnailRef;

	typedef enum {
 		OR_ERROR_NONE = 0,
		OR_ERROR_BUF_TOO_SMALL = 1,
		OR_ERROR_NOTAREF = 2,
		OR_ERROR_LAST_ 
	} or_error;

	typedef enum {
		OR_THUMB_SIZE_NONE = 0,
		OR_THUMB_SIZE_SMALL,
		OR_THUMB_SIZE_LARGE
	} or_thumb_size;


	typedef enum {
		OR_DATA_TYPE_NONE = 0,
		OR_DATA_TYPE_PIXMAP,
		OR_DATA_TYPE_JPEG,
		
		OR_DATA_TYPE_UNKNOWN
	} or_data_type;

	/*! Extract thumbnail for raw file

	\param filename the file name to extract from
	\param preferred_size preferred thumnail size
	\param thumb the thumbnail object ref to store it in
	If the ref is NULL, then a new one is allocated. It is
	the responsibility of the caller to release it.
	\return error code
	 */
	or_error openraw_get_extract_thumbnail(const char* filename,
					 or_thumb_size preferred_size,
					 ORThumbnailRef *thumb);
	
	/*! Allocate a thumbnail
	 */
	ORThumbnailRef openraw_thumbnail_new(void);

	/*! Release a thumbnail object
	 */
	or_error openraw_thumbnail_release(ORThumbnailRef thumb);

#ifdef __cplusplus
};
#endif

#endif
