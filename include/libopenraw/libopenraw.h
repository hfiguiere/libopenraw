




#ifndef __LIBOPENRAW_H__
#define __LIBOPENRAW_H__

#include <libopenraw/io.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
 		OR_ERROR_NONE = 0,
		OR_ERROR_BUF_TOO_SMALL = 1,
		
	} or_error;

	typedef enum {
		OR_THUMB_SIZE_NONE = 0,
		OR_THUMB_SIZE_SMALL,
		OR_THUMB_SIZE_LARGE
	} or_thumb_size;

	/*! Extract thumbnail for raw file

	\param filename the file name to extract from
	\param preferred_size preferred thumnail size
	\param buf buffer to store the thumnail
	\param buf_size size of the buffer. On exit, returns
	the size actuall fed OR the need size if 
	OR_ERROR_BUF_TOO_SMALL
	\return error code

	Returns OR_ERROR_BUF_TOO_SMALL if the passed buffer 
	size is too small. In that case, buf_size will contain 
	the size required.
	 */
	or_error openraw_get_extract_thumbnail(const char* filename,
					 or_thumb_size preferred_size,
					 void *buf, size_t *buf_size);

	

#ifdef __cplusplus
};
#endif

#endif
