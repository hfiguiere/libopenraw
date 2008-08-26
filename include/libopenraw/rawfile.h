/*
 * libopenraw - rawfile.h
 *
 * Copyright (C) 2007-2008 Hubert Figuiere
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef __LIBOPENRAW_RAWFILE_H_
#define __LIBOPENRAW_RAWFILE_H_

#include <libopenraw/types.h>
#include <libopenraw/consts.h>
#include <libopenraw/rawdata.h>
#include <libopenraw/thumbnails.h>
#include <libopenraw/metadata.h>
#include <libopenraw/bitmapdata.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RawFile *ORRawFileRef;

/** return a NULL terminated list of extensions.
 * that the library supposedly handle.
 * @return a NULL terminated list. Belongs to the library.
 */
const char **
or_get_file_extensions();

ORRawFileRef
or_rawfile_new(const char* filename, or_rawfile_type type);

ORRawFileRef
or_rawfile_new_from_memory(const uint8_t *buffer, uint32_t len, or_rawfile_type type);

or_error
or_rawfile_release(ORRawFileRef rawfile);

or_rawfile_type
or_rawfile_get_type(ORRawFileRef rawfile);

/** return the typeid to identify the exact file type */
or_rawfile_typeid
or_rawfile_get_typeid(ORRawFileRef rawfile);

or_error
or_rawfile_get_thumbnail(ORRawFileRef rawfile, uint32_t preferred_size,
						 ORThumbnailRef thumb);

or_error
or_rawfile_get_rawdata(ORRawFileRef rawfile, ORRawDataRef rawdata, 
						   uint32_t options);

/** Get the rendered image from the raw data 
 * @param rawdata the preallocated bitmap data.
 * @param options option for rendering. Pass 0 for now.
 */
or_error
or_rawfile_get_rendered_image(ORRawFileRef rawfile, ORBitmapDataRef rawdata,
			      uint32_t options);


/** Get the orientation, This is a convenience method.
 * @param rawfile the RAW file object.
 * @return the orienation using EXIF semantics. If
 * there is no orientation attribute, return 0.
 */
int32_t
or_rawfile_get_orientation(ORRawFileRef rawfile);

#if 0
/** Get the metadata value
 * @param rawfile the RAW file object.
 * @param meta_index the index value which is NS | index
 */
ORConstMetaValueRef
or_rawfile_get_metavalue(ORRawFileRef rawfile, int32_t meta_index);

/** Get the metadata out of the raw file as XMP
 * @param rawfile the rawfile object
 * @return the XMP meta. It belongs to the rawfile.
 * Use Exempi to deal with the content.
 */
XmpPtr
or_rawfile_get_xmp(ORRawFileRef rawfile);

#endif

#ifdef __cplusplus
}
#endif

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
