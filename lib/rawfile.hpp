/* -*- Mode: C++ -*- */
/*
 * libopenraw - rawfile.hpp
 *
 * Copyright (C) 2005-2020 Hubert Figuière
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

#pragma once

#include <string>
#include <vector>

#include <libopenraw/rawfile.h>

#include "ifddir.hpp"
#include "makernotedir.hpp"

/** @defgroup internals libopenraw Internals
 *
 * The libopenraw internals include the C++ API that is bound by
 * the public C API.
 *
 * @{
 */
/** @brief Global namespace for libopenraw
 *
 * This namespace exposes the C++ API that the C API will bind.
 */
namespace OpenRaw {

class Thumbnail;
class RawData;
class BitmapData;
class MetaValue;
class MetadataIterator;

/** @brief Internal classes for libopenraw */
namespace Internals {
class RawContainer;
class ThumbDesc;
/** The map between vendor model ID and TypeId */
typedef std::map<uint32_t, ::or_rawfile_typeid> ModelIdMap;
struct BuiltinColourMatrix;
template<typename T>
void audit_coefficients();
}

void init();

/** @brief RAW file. */
class RawFile
{
    template<typename T>
    friend void Internals::audit_coefficients();

public:
    typedef ::or_rawfile_type Type;
    typedef ::or_rawfile_typeid TypeId;

    RawFile(const RawFile&) = delete;
    RawFile & operator=(const RawFile &) = delete;

    /** @brief Return a NULL terminated list of file extensions
     * that the library handle.
     *
     * This is purely informational.
     * @return a pointer the list, NULL terminated. The pointer is
     * owned by the library.
     */
    static const char **fileExtensions();

    /** @brief Factory method to create the proper RawFile instance.
     * @param _filename the name of the file to load
     * @param _typeHint a hint on the type. Use UNKNOWN_TYPE
     * if you want to let the library detect it for you.
     */
    static RawFile *newRawFile(const char*_filename,
                               Type typeHint = OR_RAWFILE_TYPE_UNKNOWN);
    /** @brief Factory method to create the proper RawFile instance
     *  from content
     * @param buffer the buffer to examine.
     * @param len the number of bytes in the length.
     * @param _typeHint a hint on the type. Use UNKNOWN_TYPE
     * if you want to let the library detect it for you.
     */
    static RawFile *newRawFileFromMemory(const uint8_t *buffer, uint32_t len,
                                         Type typeHint = OR_RAWFILE_TYPE_UNKNOWN);

    /** @brief Destructor */
    virtual ~RawFile();
    /** @brief Accessor for the type */
    Type type() const;

    /** @brief The RAW file type ID. Identify it if needed.
     *  @todo figure how to make this const.
     */
    TypeId typeId();
    TypeId vendorId();

    // standard api, like get thumbnail
    // and get exif.

    /** @brief List the available thumbnail sizes
     */
    const std::vector<uint32_t> & listThumbnailSizes(void);
    /** @brief Get the thumbnail from the raw file
     * @param size the square size in px
     * @param [out] thumbnail the thumbnail to extract into
     * @return the error code
     */
    ::or_error getThumbnail(uint32_t size, Thumbnail & thumbnail);

    /** @brief Get the RAW data
     * @param rawdata the RawData to put the data into
     * @param options the option bits defined by %or_options
     * @return the error code
     */
    ::or_error getRawData(RawData & rawdata, uint32_t options);

    /** @brief Get the rendered image
     * @param bitmapdata the BitmapData to put the image into
     * @param options the option bits. Pass 0 for now.
     * @return the error code
     */
    ::or_error getRenderedImage(BitmapData & bitmapdata, uint32_t options);

    /** @brief Get the orientation of the image, using Exif enums.
     */
    uint32_t getOrientation();

    /**
     * @return the number of items in the colour matrix.
     */
    uint32_t colourMatrixSize();

    /** @brief Get colour matrix
     * @param index The matrix index.
     * @param [out] matrix pointer to array of %size double.
     * @param size the size of the buffer. On out the actual size. If it is too
     * small the size is adjusted and an error %OR_ERROR_BUF_TOO_SMALL returned.
     * @return an error code.
     */
    ::or_error getColourMatrix1(double* matrix, uint32_t& size);
    ::or_error getColourMatrix2(double* matrix, uint32_t& size);

    /** @brief Get the calibration illuminant that match the colour matrix.
     * @return the Exif value. 0 = unknown. Likely not found.
     */
    ExifLightsourceValue getCalibrationIlluminant1();
    ExifLightsourceValue getCalibrationIlluminant2();

    /**
     * @brief Get the origin of the colour matrix for the RAW file
     * @return value of `or_colour_matrix_origin`
     */
    virtual or_colour_matrix_origin getColourMatrixOrigin() const;

    /** @brief Get the IFD containing the CFA */
    Internals::IfdDir::Ref cfaIfd();
    /** @brief Get the main IFD */
    Internals::IfdDir::Ref mainIfd();
    /** @brief Get the Exif IFD */
    Internals::IfdDir::Ref exifIfd();
    /** @brief Get the MakerNote IFD */
    Internals::MakerNoteDir::Ref makerNoteIfd();

    /** @brief Get a metadata value
     *
     * @return A MetaValue, or NULL if not found.
     */
    const MetaValue *getMetaValue(int32_t meta_index);

    MetadataIterator* getMetadataIterator();
protected:
    /** @brief Locate the IFD for the raw data
     *
     * This is not necessarily a unique IFD and it can be the same as
     * the main.
     *
     * @return the CFA Ifd. May be null.
     */
    virtual Internals::IfdDir::Ref _locateCfaIfd() = 0;
    /** @brief Locate the main IFD
     *
     * @return the main IFD. Main be null.
     */
    virtual Internals::IfdDir::Ref _locateMainIfd() = 0;
    /** @brief Locate the Exif IFD.
     *
     * The default implementation follow the specification by
     * by calling getExifIFD() on the main IFD.
     *
     * @return the Exif IFD.
     */
    virtual Internals::IfdDir::Ref _locateExifIfd();
    /** @brief Locate the MakerNote IFD.
     *
     * The default implementation follow the specification by
     * by calling getMakerNoteIfd() on the main IFD.
     *
     * @return the MakerNote IFD.
     */
    virtual Internals::MakerNoteDir::Ref _locateMakerNoteIfd();

    struct camera_ids_t {
        const char * model;
        const uint32_t type_id;
    };
    /**
     * @brief Construct a raw file
     * @param _type the type
     */
    RawFile(Type _type);

    /** @brief Helper to get the TypeId from the map
     * @return the TypeId or 0
     */
    static RawFile::TypeId modelid_to_typeid(const std::map<uint32_t, RawFile::TypeId>& model_map,
                                             uint32_t model_id);
    /** @brief Get the vendor camera ID location.
     * @param ifd the IfdDir where it is.
     * @param index the value index in the IfdDir.
     * @param model_map a point to the model map. Can't be null.
     * @return true if there is one, otherwise false
     */
    virtual bool vendorCameraIdLocation(Internals::IfdDir::Ref& ifd, uint16_t& index,
                                        const Internals::ModelIdMap*& model_map);
    /** @brief Set the file type id */
    void _setTypeId(TypeId _type_id);
    /** @brief Just get the type id value. No identification.
     *  You might want to use typeId() in the general case.
     */
    TypeId _typeId() const;

    /** @brief Get the container. */
    virtual Internals::RawContainer* getContainer() const = 0;

    /** @brief Enumerate the thumbnail sizes.
     * @param list the list to enumerate into
     * @return OR_ERROR_NONE if success
     */
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) = 0;

    /** @brief Get the thumbnail of exact size.
     * @param size the size in pixel of the square
     * @param [out] thumbnail the thumbnail to load
     * @return OR_ERROR_NONE if success
     * @see listThumbnailSizes() to understand how to fetch the sizes
     * available
     */
    virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail);
    void _addThumbnail(uint32_t size, Internals::ThumbDesc&& desc);

    /** @brief Get the RAW data
     * @param data The RAW data
     * @param option The option bits
     * @return OR_ERROR_NONE if success
     * Return the data compressed or uncompressed.
     */
    virtual ::or_error _getRawData(RawData & data, uint32_t options) = 0;

    /** @brief Get the colour matrix.
     * @param index 1 or 2
     */
    virtual ::or_error _getColourMatrix(uint32_t index, double* matrix, uint32_t & size);
    virtual ExifLightsourceValue _getCalibrationIlluminant(uint16_t index);
    /** @brief Implementation for getMetaValue() */
    virtual MetaValue *_getMetaValue(int32_t /*meta_index*/) = 0;

    TypeId _typeIdFromModel(const std::string& make, const std::string & model);
    TypeId _typeIdFromMake(const std::string& make, const std::string& model);
    void _setIdMap(const camera_ids_t *map);
    void _setMatrices(const Internals::BuiltinColourMatrix* matrices);
    const Internals::BuiltinColourMatrix* _getMatrices() const;

    /** @brief Identify the file and set the ID internally. */
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
    static ::or_error identifyIOBuffer(IO::Stream::Ptr& stream,
                                       RawFile::Type& _type);
    static const camera_ids_t s_make[];
    static const camera_ids_t* _lookupCameraId(const camera_ids_t * map,
                                               const std::string& value);
    static const camera_ids_t* lookupVendorId(const camera_ids_t * map,
                                              const std::string& value);

    class Private;

    Private *d;
};

}

/** @} */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  c-basic-offset: 4
  tab-width: 4
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
