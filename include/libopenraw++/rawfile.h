/*
 * libopenraw - rawfile.h
 *
 * Copyright (C) 2005-2006 Hubert Figuiere
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



#ifndef __RAWFILE_H
#define __RAWFILE_H

#include <string>
#include <vector>

#include <libopenraw/libopenraw.h>

namespace OpenRaw {

namespace IO {
class Stream;
}
class Thumbnail;
class RawData;
class BitmapData;
class MetaValue;

void init();

class RawFile
{
public:
    typedef ::or_rawfile_type Type;
    typedef ::or_rawfile_typeid TypeId;


    /** return a NULL terminated list of file extensions 
     * that the library handle. This is purely informational.
     * @return a pointer the list, NULL terminated. The pointer is
     * owned by the library.
     */
    static const char **fileExtensions();

    /** factory method to create the proper RawFile instance.
     * @param _filename the name of the file to load
     * @param _typeHint a hint on the type. Use UNKNOWN_TYPE
     * if you want to let the library detect it for you.
     */
    static RawFile *newRawFile(const char*_filename, 
                               Type _typeHint = OR_RAWFILE_TYPE_UNKNOWN);
    /** factory method to create the proper RawFile instance 
     *  from content 
     * @param buffer the buffer to examine.
     * @param len the number of bytes in the length.
     * @param _typeHint a hint on the type. Use UNKNOWN_TYPE
     * if you want to let the library detect it for you.
     */
    static RawFile *newRawFileFromMemory(const uint8_t *buffer, uint32_t len, 
                                         Type _typeHint = OR_RAWFILE_TYPE_UNKNOWN);

    /** Destructor */
    virtual ~RawFile();
    /** Accessor for the type */
    Type type() const;

    /** The RAW file type ID. Identify it if needed. 
     *  @todo figure how to make this const.
     */
    TypeId typeId();

    // standard api, like get thumbnail
    // and get exif.

    /** list the available thumbnail sizes
     */
    const std::vector<uint32_t> & listThumbnailSizes(void);
    /** Get the thumbnail from the raw file 
     * @param size the square size in px
     * @param thumbnail the thumbnail to extract into
     * @return the error code
     */
    ::or_error getThumbnail(uint32_t size, Thumbnail & thumbnail);

    /** Get the RAW data 
     * @param rawdata the RawData to put the data into
     * @param options the option bits defined by %or_options
     * @return the error code
     */
    ::or_error getRawData(RawData & rawdata, uint32_t options);

    /** Get the rendered image
     * @param bitmapdata the BitmapData to put the image into
     * @param options the option bits. Pass 0 for now.
     * @return the error code
     */
    ::or_error getRenderedImage(BitmapData & bitmapdata, uint32_t options);    

    /** Get the orientation of the image, using Exif enums.
     */
    int32_t getOrientation();

    const MetaValue *getMetaValue(int32_t meta_index);
protected:
    struct camera_ids_t {
        const char * model;
        const uint32_t type_id;
    };
    /** 
     * Construct a raw file
     * @param s the stream to load from. Take ownership.
     * @param _type the type
     */
    RawFile(IO::Stream *s, Type _type);

    /** Set the file type id */
    void _setTypeId(TypeId _type_id);

    /** enumerate the thumbnail sizes. 
     * @param list the list to enumerate into
     * @return OR_ERROR_NONE if success
     */
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) = 0;
		
    /** get the thumbnail of exact size. 
     * @param size the size in pixel of the square
     * @retval thumbnail the thumbnail to load
     * @return OR_ERROR_NONE if success
     * @seealso listThumbnailSizes() to understand how to fetch the sizes
     * available
     */
    virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail) = 0;
    /** get the RAW data 
     * @param data the RAW data
     * @param option the option bits
     * @return OR_ERROR_NONE if success
     * Return the data compressed or uncompressed.
     */
    virtual ::or_error _getRawData(RawData & data, uint32_t options) = 0;

    virtual MetaValue *_getMetaValue(int32_t /*meta_index*/) = 0;

    TypeId _typeIdFromModel(const std::string & model);
    void _setIdMap(const camera_ids_t *map);
    virtual void _identifyId() = 0;
private:
    static Type identify(const char*_filename);
    static ::or_error identifyBuffer(const uint8_t* buff, size_t len,
                                     Type &_type);


    RawFile(const RawFile&);
    RawFile & operator=(const RawFile &);

    class Private;

    Private *d;
};



}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
#endif
