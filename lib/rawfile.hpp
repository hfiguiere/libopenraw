/* -*- Mode: C++ -*- */
/*
 * libopenraw - rawfile.h
 *
 * Copyright (C) 2005-2016 Hubert Figuière
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



#ifndef LIBOPENRAWPP_RAWFILE_H_
#define LIBOPENRAWPP_RAWFILE_H_

#include <string>
#include <vector>

#include <libopenraw/rawfile.h>

namespace OpenRaw {

namespace IO {
class Stream;
}
class Thumbnail;
class RawData;
class BitmapData;
class MetaValue;

namespace Internals {
class RawContainer;
class ThumbDesc;
struct BuiltinColourMatrix;
}

void init();

class RawFile
{
public:
    typedef ::or_rawfile_type Type;
    typedef ::or_rawfile_typeid TypeId;

    RawFile(const RawFile&) = delete;
    RawFile & operator=(const RawFile &) = delete;

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

    /**
     * @return the number of items in the colour matrix.
     */
    uint32_t colourMatrixSize();

    /** Get colour matrix
     * @param index The matrix index.
     * @param [out] matrix an array of %size double.
     * @param size the size of the buffer. On out the actual size. If it is too 
     * small the size is adjusted and an error %OR_ERROR_BUF_TOO_SMALL returned.
     * @return an error code.
     */
    ::or_error getColourMatrix1(double* matrix, uint32_t & size);
    ::or_error getColourMatrix2(double* matrix, uint32_t & size);

    /** Get calibration illuminant that match the colour matrix.
     * @return the Exif value. 0 = unknown. Likely not found.
     */
    ExifLightsourceValue getCalibrationIlluminant1();
    ExifLightsourceValue getCalibrationIlluminant2();

    const MetaValue *getMetaValue(int32_t meta_index);
protected:
    struct camera_ids_t {
        const char * model;
        const uint32_t type_id;
    };
    /**
     * Construct a raw file
     * @param _type the type
     */
    RawFile(Type _type);

    /** Set the file type id */
    void _setTypeId(TypeId _type_id);
    /** Just get the type id value. No identification.
     *  You might want to use %typeId() in the general case.
     */
    TypeId _typeId() const;

    /** Get the container. */
    virtual Internals::RawContainer* getContainer() const = 0;

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
    virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail);
    void _addThumbnail(uint32_t size, const Internals::ThumbDesc& desc);

    /** get the RAW data 
     * @param data the RAW data
     * @param option the option bits
     * @return OR_ERROR_NONE if success
     * Return the data compressed or uncompressed.
     */
    virtual ::or_error _getRawData(RawData & data, uint32_t options) = 0;

    /** get the colour matrix.
     * @param index 1 or 2
     */
    virtual ::or_error _getColourMatrix(uint32_t index, double* matrix, uint32_t & size);
    virtual ExifLightsourceValue _getCalibrationIlluminant(uint16_t index);
    virtual MetaValue *_getMetaValue(int32_t /*meta_index*/) = 0;

    TypeId _typeIdFromModel(const std::string& make, const std::string & model);
    TypeId _typeIdFromMake(const std::string& make);
    void _setIdMap(const camera_ids_t *map);
    void _setMatrices(const Internals::BuiltinColourMatrix* matrices);
    const Internals::BuiltinColourMatrix* _getMatrices() const;

    virtual void _identifyId() = 0;

    static ::or_error _getBuiltinLevels(const Internals::BuiltinColourMatrix* m,
                                        TypeId type_id,
                                        uint16_t & black,
                                        uint16_t & white);
    static ::or_error _getBuiltinColourMatrix(const Internals::BuiltinColourMatrix* m,
                                              TypeId type_id,
                                              double* matrix,
                                              uint32_t & size);

private:
    static Type identify(const char*_filename);
    static ::or_error identifyBuffer(const uint8_t* buff, size_t len,
                                     Type &_type);
    static const camera_ids_t s_make[];
    static const camera_ids_t* _lookupCameraId(const camera_ids_t * map,
                                               const std::string& value);


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
