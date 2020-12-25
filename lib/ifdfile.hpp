/* -*- Mode: C++ -*- */
/*
 * libopenraw - ifdfile.hpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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

#include <stdint.h>
#include <sys/types.h>

#include <vector>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "io/stream.hpp"
#include "ifd.hpp"
#include "rawcontainer.hpp"
#include "ifddir.hpp"
#include "makernotedir.hpp"

namespace OpenRaw {

class MetaValue;
class RawData;

namespace Internals {
/** @addtogroup ifd_parsing
 * @{
 */

class IfdFileContainer;

/** @brief A generic IFD (TIFF) based raw file. */
class IfdFile : public OpenRaw::RawFile {
#if defined(IN_TESTSUITE)
public:
    friend class ::Test; // for testing
#endif

protected:
    IfdFile(const IO::Stream::Ptr &s, Type _type,
            bool instantiateContainer = true);
    virtual ~IfdFile();

    /** @brief List the thumbnails in the IFD
     * @retval list the list of thumbnails
     * @return the error code. OR_ERROR_NOT_FOUND if no
     * thumbnail are found.
     */
    virtual ::or_error _enumThumbnailSizes(
        std::vector<uint32_t> &list) override;

    /** @brief Add a thumbnail from a stream
     * @param offset the offset in the IO stream
     * @param len the length of the data
     */
    ::or_error _addThumbnailFromStream(uint32_t offset, uint32_t len,
                                       std::vector<uint32_t>& list);
    /** @brief Add the thumbnail found in the IfdEntry
     * @param offset is the offset for MakerNote. Pass 0 if not.
     * @param list the thumbnail list to add to.
     */
    ::or_error _addThumbnailFromEntry(const IfdEntry::Ref& e, off_t offset,
                                      std::vector<uint32_t>& list);

    /** @brief Locate the thumnail in the IFD
     * @param dir the IfdDir where to locate the thumbnail
     * @return the error code. OR_ERROR_NOT_FOUND if the
     * thumbnails are not found.
     */
    virtual ::or_error _locateThumbnail(const IfdDir::Ref &dir,
                                        std::vector<uint32_t> &list);
    /** load the compressed rawdata from a standard location in an IFD
     * @param data the data storage
     * @param dir the IFD
     * @return the error code.
     */
    ::or_error _getRawDataFromDir(RawData &data, const IfdDir::Ref &dir);

    /** @brief Get the JPEG thumbnail offset from dir.
     *
     * @param dir the IFD to get the thumbnail from
     * @param len the length of the JPEG stream. 0 is not valid.
     * @return the offset. 0 is not valid.
     */
    virtual uint32_t _getJpegThumbnailOffset(const IfdDir::Ref &dir,
                                             uint32_t &len);

    IO::Stream::Ptr m_io;          /**< the IO handle */
    IfdFileContainer *m_container; /**< the real container */

    /** @inherit */
    virtual RawContainer *getContainer() const override;

    /** @inherit */
    virtual IfdDir::Ref _locateCfaIfd() override;
    /** @inherit */
    virtual IfdDir::Ref _locateMainIfd() override;

    /** @inherit */
    virtual void _identifyId() override;

    /** @inherit */
    virtual MetaValue *_getMetaValue(int32_t meta_index) override;

    /** @brief Translate the compression type from the tiff type (16MSB)
     * to the RAW specific type if needed (16MSB)
     *
     * @param tiffCompression the 16 bits value from TIFF
     * @return the actually value. Anything >= 2^16 is specific the RAW type
     */
    virtual uint32_t _translateCompressionType(
        IFD::TiffCompress tiffCompression);

    /** @brief Unpack the data
     *
     * @param bpc bits per components
     * @param compression the compression type
     * @param x the width
     * @param y the height
     * @param offset the offset of the data
     * @param byte_length the amount of data
     * @return error code
     */
    virtual ::or_error _unpackData(uint16_t bpc, uint32_t compression,
                                   RawData &data, uint32_t x, uint32_t y,
                                   uint32_t offset, uint32_t byte_length);

    /** @inherit */
    virtual ::or_error _getRawData(RawData &data, uint32_t options) override;
    /** @brief Call to decompress if needed from _getRawData() */
    virtual ::or_error _decompressIfNeeded(RawData &, uint32_t);

private:

    IfdFile(const IfdFile &) = delete;
    IfdFile &operator=(const IfdFile &) = delete;
};
/** @} */
}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
