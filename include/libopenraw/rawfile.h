/*
 * libopenraw - rawfile.h
 *
 * Copyright (C) 2007-2020 Hubert Figuière
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


#ifndef LIBOPENRAW_RAWFILE_H_
#define LIBOPENRAW_RAWFILE_H_

#include <libopenraw/types.h>
#include <libopenraw/consts.h>
#include <libopenraw/rawdata.h>
#include <libopenraw/thumbnails.h>
#include <libopenraw/metadata.h>
#include <libopenraw/bitmapdata.h>

/** @defgroup raw_file_api RawFile API
 * @ingroup public_api
 *
 * @brief Decode the raw file
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Return a NULL terminated list of extensions that the library supposedly handle.
 *
 * @return A NULL terminated list. Owned the library.
 */
const char **
or_get_file_extensions();

/** @brief Create a new %RawFile object from a file.
 * @param filename The path to the file to open.
 * @param type The hint for the file type. Pass %OR_RAWFILE_TYPE_UNKNOWN to let the library
 * guess.
 * @return A new allocated RawFile pointer. Must be freed with %or_rawfile_release().
 */
ORRawFileRef
or_rawfile_new(const char* filename, or_rawfile_type type);

/** @brief Create a new %RawFile object from a memory buffer.
 * @param buffer The memory buffer: bytes from the RAW file.
 * @param len The length of the memory buffer in bytes.
 * @param type The hint for the file type. Pass %OR_RAWFILE_TYPE_UNKNOWN to let the library
 * guess.
 * @return A new allocated RawFile pointer. Must be freed with %or_rawfile_release().
 */
ORRawFileRef
or_rawfile_new_from_memory(const uint8_t *buffer, uint32_t len, or_rawfile_type type);

/** @brief Release the %RawFile.
 * @param [in] rawfile The %RawFile object to release.
 * @return An error code. %OR_ERROR_NOT_AREF if the pointer is NULL.
 */
or_error or_rawfile_release(ORRawFileRef rawfile);

/** @brief Get the %RawFile type
 *
 * @return The type from %or_rawfile_type. It isn't necessarily what was passed
 * at creation time.
 */
or_rawfile_type or_rawfile_get_type(ORRawFileRef rawfile);

/** @brief Return the type id to identify the exact file type.
 *
 * @return The type ID. It is a combination of vendor ID and camera ID.
 *
 * @see %or_rawfile_typeid.
 */
or_rawfile_typeid or_rawfile_get_typeid(ORRawFileRef rawfile);

/** @brief Return the type id to identify the vendor.
 *
 * @return The vendor ID. Use the constants values to match.
 */
or_rawfile_typeid or_rawfile_get_vendorid(ORRawFileRef rawfile);

/** @brief Get the the array of thumbnail sizes.
 *
 * @param rawfile The RawFile.
 * @param [out] size The size of the array is returned
 * @return The array. It is owned by the raw file. Or %nullptr in case of error.
 * */
const uint32_t* or_rawfile_get_thumbnail_sizes(ORRawFileRef  rawfile, size_t* size);

/** @brief Get a thumbnail from a RawFile..
 *
 * Get a thumbnail close to the preferred size. If there is no exact match, it
 * will prefer a bigger thumbnail so that you can downsize it.
 *
 * Return an error in case or error. %OR_ERROR_NOT_FOUND if no thumbnail can be
 * found.
 *
 * @param rawfile The RawFile object.
 * @param preferred_size The requested preferred size.
 * @param [out] An error code. %OR_ERROR_NONE in case of success.
 * @return The Thumbnail object to store the data. Must be freed.
 */
ORThumbnailRef
or_rawfile_get_thumbnail(ORRawFileRef rawfile, uint32_t preferred_size, or_error *error);

/** @brief Get the RawData out of the RawFile.
 *
 * Will return an error code: %OR_ERROR_NOT_FOUND if the RAW data can't be
 * located. This likely indicate a file that isn't properly supported.
 *
 * The RawData object will contain the uncompress RAW data if possible (unless
 * otherwise requested).
 *
 * @param rawfile The RawFile.
 * @param options Some options. Pass %OR_OPTIONS_DONT_DECOMPRESS if
 * you don't want the RAW data stream to be decompressed, %OR_OPTIONS_NONE otherwise.
 * @param error The error code. Pass %nullptr if not desired.
 * @return An %ORRawDataRef or %nullptr in case of error.
 */
ORRawDataRef
or_rawfile_get_rawdata(ORRawFileRef rawfile, uint32_t options, or_error *error);

/** @brief Get the rendered image from the raw file
 * @param rawfile The raw file.
 * @param options Option for rendering. Pass %OR_OPTIONS_NONE for now.
 * @param [out] error An error code. %OR_ERROR_NOTAREF is %rawfile is NULL.
 * @return The rendered bitmap %ORBitmapDataRef
 */
ORBitmapDataRef
or_rawfile_get_rendered_image(ORRawFileRef rawfile, uint32_t options, or_error *error);


/** @brief Get the orientation.
 *
 * This is a convenince method, equivalent to getting the value of
 * %EXIF_TAG_ORIENTATION.
 *
 * @param rawfile The RawFile object.
 * @return the orientation using EXIF semantics. If there is no orientation
 * attribute, return 0.
 */
int32_t
or_rawfile_get_orientation(ORRawFileRef rawfile);

/** @brief Get the first colour matrix.
 *
 *  The error code will be one of the following: %OR_ERROR_BUF_TOO_SMALL if
 *  %matrix is too small. Check the value of %size to know how much you need.
 *  %OR_ERROR_NOT_IMPLEMENTED if there is no matrix in the file nor built-in
 *  matrices.
 *
 *  Call %or_rawfile_get_colour_matrix_origin() if you want to know if it is a built-in
 *  matrix.
 *
 *  @see %or_rawfile_get_colour_matrix_origin()
 *
 *  @param rawfile The RAW file object
 *  @param [int] matrix The storage array for the matrix
 *  @param [in/out] size The size of the %matrix array. On output the actual size of the matrix.
 *  @return An error codex.
 */
or_error
or_rawfile_get_colourmatrix1(ORRawFileRef rawfile, double* matrix, uint32_t* size);

/** @brief Get the second colour matrix.
 *
 * See %or_rawfile_get_colourmatrix1 for details. Will return %OR_ERROR_INVALID_PARAM if the
 * matrix doesn't exist in the file. There won't be a built-in matrix.
 *
 * @see %or_rawfile_get_colourmatrix1()
 */
or_error
or_rawfile_get_colourmatrix2(ORRawFileRef rawfile, double* matrix, uint32_t* size);

/** @brief Get calibration illuminant for the first colour matrix.
 *
 * @return The Exif value. See %ExifLightsourceValue. %EV_LIGHTSOURCE_UNKNOWN means the
 * matrix is not found.
 *
 * @see %ExifLightsourceValue.
 */
uint32_t or_rawfile_get_calibration_illuminant1(ORRawFileRef rawfile);

/** @brief Get calibration illuminant for the second colour matrix.
 *
 * @see %or_rawfile_get_calibration_illuminant1
 */
uint32_t or_rawfile_get_calibration_illuminant2(ORRawFileRef rawfile);

/** @brief Get the colour matrix origin for file.
 *
 *  This allow to determine if it is provided by the file or as a hardcoded
 *  value in the library.
 *
 *  @param rawfile The RawFile object
 *  @return The colour matrix origin
 */
or_colour_matrix_origin
or_rawfile_get_colour_matrix_origin(ORRawFileRef rawfile);

/** @brief Get the metadata value
 * @param rawfile the RawFile object.
 * @param key the string key for the value
 * @return a MetaValue that must be freed with %or_metavalue_release
 */
ORMetaValueRef
or_rawfile_get_metavalue(ORRawFileRef rawfile, const char* key);

/** @brief Get an IFD directory
 *
 * @param rawfile The %RawFile object.
 * @param ifd The IFD you want, from %or_ifd_dir_type.
 * @return An IfdDir. Owned by the raw file.
 *
 * @see %or_ifd_dir_type
 */
ORIfdDirRef
or_rawfile_get_ifd(ORRawFileRef rawfile, or_ifd_dir_type ifd);

/** @brief Get a metadata iterator.
 *
 * @param rawfile The RawFile object.
 * @return The metadata iterator. Must be freed with %or_metadata_iterator_free()
 */
ORMetadataIteratorRef
or_rawfile_get_metadata_iterator(ORRawFileRef rawfile);

#if 0
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

/** @} */

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
