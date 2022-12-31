/* -*- Mode: C++ -*- */
/*
 * libopenraw - ciffcontainer.hpp
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

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include <vector>
#include <memory>

#include <libopenraw/debug.h>

#include "io/stream.hpp"
#include "ciff/heap.hpp"
#include "rawcontainer.hpp"
#include "trace.hpp"

namespace OpenRaw {
namespace Internals {

class CIFFContainer;

/**
 * @brief CIFF is the container for CRW files. It is an attempt from Canon to
 * make this a standard. I guess it failed.
 */
namespace CIFF {

/** @brief Tags for the CIFF records.
 *
 * List made by a combination of the CIFF spec and
 * what exifprobe by Duane H. Hesser has.
 */
enum {
    TAG_NULLRECORD =  0x0000,
    TAG_FREEBYTES = 0x0001,
    TAG_COLORINFO1 = 0x0032,
    TAG_FILEDESCRIPTION = 0x0805,
    TAG_RAWMAKEMODEL = 0x080a,
    TAG_FIRMWAREVERSION = 0x080b,
    TAG_COMPONENTVERSION = 0x080c,
    TAG_ROMOPERATIONMODE = 0x080d,
    TAG_OWNERNAME = 0x0810,
    TAG_IMAGETYPE = 0x0815,
    TAG_ORIGINALFILENAME = 0x0816,
    TAG_THUMBNAILFILENAME = 0x0817,

    TAG_TARGETIMAGETYPE = 0x100a,
    TAG_SHUTTERRELEASEMETHOD = 0x1010,
    TAG_SHUTTERRELEASETIMING = 0x1011,
    TAG_RELEASESETTING = 0x1016,
    TAG_BASEISO = 0x101c,
    TAG_FOCALLENGTH = 0x1029,
    TAG_SHOTINFO = 0x102a,
    TAG_COLORINFO2 = 0x102c,
    TAG_CAMERASETTINGS = 0x102d,
    TAG_SENSORINFO = 0x1031,
    TAG_CUSTOMFUNCTIONS = 0x1033,
    TAG_PICTUREINFO = 0x1038,
    TAG_WHITEBALANCETABLE = 0x10a9,
    TAG_COLORSPACE = 0x10b4,

    TAG_IMAGESPEC = 0x1803,
    TAG_RECORDID = 0x1804,
    TAG_SELFTIMERTIME = 0x1806,
    TAG_TARGETDISTANCESETTING = 0x1807,
    TAG_SERIALNUMBER = 0x180b,
    TAG_CAPTUREDTIME = 0x180e,
    TAG_IMAGEINFO = 0x1810,
    TAG_FLASHINFO = 0x1813,
    TAG_MEASUREDEV = 0x1814,
    TAG_FILENUMBER = 0x1817,
    TAG_EXPOSUREINFO = 0x1818,
    TAG_DECODERTABLE = 0x1835,

    TAG_RAWIMAGEDATA = 0x2005,
    TAG_JPEGIMAGE = 0x2007,
    TAG_JPEGTHUMBNAIL = 0x2008,

    TAG_IMAGEDESCRIPTION = 0x2804,
    TAG_CAMERAOBJECT = 0x2807,
    TAG_SHOOTINGRECORD = 0x3002,
    TAG_MEASUREDINFO = 0x3003,
    TAG_CAMERASPECIFICATION = 0x3004,
    TAG_IMAGEPROPS = 0x300a,
    TAG_EXIFINFORMATION = 0x300b
};

class Heap;

/** @brief Camera settings are stored as array of 16-bits int */
typedef std::vector<uint16_t> CameraSettings;

/** @brief ImageSpec struct from CIFF.
 */
class ImageSpec
{
public:
    ImageSpec()
        : imageWidth(0), imageHeight(0),
          pixelAspectRatio(0), rotationAngle(0),
          componentBitDepth(0), colorBitDepth(0),
          colorBW(0)
        {
        }

    /** read the struct from container
     * @param offset the offset to read from, relative
     * to the begining of the container.
     * @param container the container to read from.
     */
    bool readFrom(off_t offset, CIFFContainer *container);
    /** Return the EXIF orientation (EXIF value).
     */
    int32_t exifOrientation() const;

    uint32_t imageWidth; /**< Width (horizontal) in pixel. */
    uint32_t imageHeight; /**< Height (vertical) in pixel. */
    uint32_t /*float32*/pixelAspectRatio; /**< Pixel aspect ratio. */
    int32_t rotationAngle; /**< Rotation angle in degrees. */
    uint32_t componentBitDepth; /**< bit depth per component. */
    uint32_t colorBitDepth; /**< bit depth for colour. */
    uint32_t colorBW;  /**< colour or B&W. See CIFF spec. */
};



} /* namespace CIFF */

/** CIFF container
 * as described by the CIFF documentation
 */
class CIFFContainer
    : public RawContainer
{
public:
    CIFFContainer(const IO::Stream::Ptr &file);
    virtual ~CIFFContainer();

    CIFFContainer(const CIFFContainer &) = delete;
    CIFFContainer & operator=(const CIFFContainer &) = delete;

    CIFF::HeapRef heap();

    const CIFF::HeapFileHeader & header() const
        {
            return m_hdr;
        }
    CIFF::HeapRef getImageProps();
    const CIFF::RecordEntry * getRawDataRecord() const;
    const CIFF::ImageSpec * getImageSpec();
    CIFF::HeapRef getCameraProps();
    CIFF::HeapRef getExifInfo();
    CIFF::CameraSettings getCameraSettings();
private:
    bool _loadHeap();
    EndianType _readHeader();

    friend class CIFF::HeapFileHeader;
    CIFF::HeapFileHeader m_hdr;
    CIFF::HeapRef m_heap;
    CIFF::HeapRef m_imageprops;
    bool m_hasImageSpec;
    CIFF::ImageSpec m_imagespec;
    CIFF::HeapRef m_cameraprops;
    CIFF::HeapRef m_exifinfo;
};


}
}
