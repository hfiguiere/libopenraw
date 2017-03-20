/* -*- Mode: C++ -*- */
/*
 * libopenraw - mrwcontainer.h
 *
 * Copyright (C) 2006-2015 Hubert Figuiere
 * Copyright (C) 2008 Bradley Broom
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

#ifndef OR_INTERNALS_MRW_CONTAINER_H_
#define OR_INTERNALS_MRW_CONTAINER_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <memory>
#include <vector>

#include "io/stream.hpp"
#include "option.hpp"
#include "rawcontainer.hpp"

#include "ifdfilecontainer.hpp"

namespace OpenRaw {
namespace Internals {

class MRWContainer;

namespace MRW {

const int DataBlockHeaderLength = 8; /* Number of bytes in a block header. */

/** Represents an MRW Data Block.
 */
class DataBlock {
public:
    typedef std::shared_ptr<DataBlock> Ref;
    typedef std::vector<Ref> RefVec;

    /** Construct a datablock from a location in the container
     * @param start the begin address relative to the container.
     * @param container the container containing the data block.
     */
    DataBlock(off_t start, MRWContainer *container);

    /** Return the offset of the data block from the begining of its container.
     */
    off_t offset() { return m_start; }

    /** Return the length of the data block, excluding the block header.
     */
    off_t length() { return m_length; }

    /** Return the name of the data block.
     */
    std::string name() {
        char id[4];
        id[0] = m_name[1];
        id[1] = m_name[2];
        id[2] = m_name[3];
        id[3] = 0;
        return std::string(id);
    }

    /** Return a signed 8-bit quantity at offset bytes from the start of the
     * data block.
     */
    Option<int8_t> int8_val(off_t offset);

    /** Return an unsigned 8-bit quantity at offset bytes from the start of the
     * data block.
     */
    Option<uint8_t> uint8_val(off_t offset);

    /** Return an unsigned 16-bit quantity at offset bytes from the start of the
     * data block.
     */
    Option<uint16_t> uint16_val(off_t offset);

    Option<std::string> string_val(off_t offset);

    bool loaded() const { return m_loaded; }

private:
    /* DRM: protection from copies. */
    DataBlock(const DataBlock &);
    DataBlock &operator=(const DataBlock &);

    off_t m_start;
    char m_name[4];
    int32_t m_length;
    MRWContainer *m_container;
    bool m_loaded;
};

/* Known offsets in PRD block.
 */
enum {
    PRD_VERSION = 0,       /* 8 chars, version string */
    PRD_SENSOR_LENGTH = 8, /* 2 bytes, Number of lines in raw data */
    PRD_SENSOR_WIDTH = 10, /* 2 bytes, Number of pixels per line */
    PRD_IMAGE_LENGTH = 12, /* 2 bytes, length of image after Divu processing */
    PRD_IMAGE_WIDTH = 14,  /* 2 bytes, width of image after Divu processing */
    PRD_DATA_SIZE = 16,  /* 1 byte,  number of bits used to store each pixel */
    PRD_PIXEL_SIZE = 17, /* 1 byte,  number of valid bits per pixel */
    PRD_STORAGE_TYPE = 18, /* 1 byte,  storage method */
    PRD_UNKNOWN1 = 19,     /* 1 byte */
    PRD_UNKNOWN2 = 20,     /* 2 bytes */
    PRD_BAYER_PATTERN = 22 /* 2 bytes, CFA pattern */
};

enum {
    STORAGE_TYPE_UNPACKED = 0x52, /* Unpacked storage (D5, D7xx) */
    STORAGE_TYPE_PACKED = 0x59    /* Packed storage (A1, A2, Maxxum/Dynax) */
};

enum {
    BAYER_PATTERN_RGGB = 0x0001,
    BAYER_PATTERN_GBRG = 0x0004 /* A200 */
};

/* Known offsets in WBG block.
 */
enum {
    WBG_DENOMINATOR_R = 0,  /* 1 byte,  log2(denominator)-6 */
    WBG_DENOMINATOR_G1 = 1, /* 1 byte,  To get actual denominator, 1<<(val+6) */
    WBG_DENOMINATOR_G2 = 2, /* 1 byte, */
    WBG_DENOMINATOR_B = 3,  /* 1 byte, */
    WBG_NOMINATOR_R = 4,    /* 2 bytes, */
    WBG_NOMINATOR_G1 = 6,   /* 2 bytes, */
    WBG_NOMINATOR_G2 = 8,   /* 2 bytes, */
    WBG_NOMINATOR_B = 10    /* 2 bytes, */
};

/* Known offsets in RIF block.
 */
enum {
    RIF_UNKNOWN1 = 0,   /* 1 byte,  */
    RIF_SATURATION = 1, /* 1 byte,  saturation setting from -3 to 3 */
    RIF_CONTRAST = 2,   /* 1 byte,  contrast setting from -3 to 3 */
    RIF_SHARPNESS =
        3, /* 1 byte,  sharpness setting from -1 (soft) to 1 (hard) */
    RIF_WHITE_BALANCE = 4,   /* 1 byte,  white balance setting */
    RIF_SUBJECT_PROGRAM = 5, /* 1 byte,  subject program setting */
    RIF_FILM_SPEED = 6,      /* 1 byte,  iso = 2^(value/8-1) * 3.125 */
    RIF_COLOR_MODE = 7,      /* 1 byte,  color mode setting */
    RIF_COLOR_FILTER = 56,   /* 1 byte,  color filter setting from -3 to 3 */
    RIF_BANDW_FILTER =
        57 /* 1 byte,  black and white filter setting from 0 to 10 */
};

enum {
    WHITE_BALANCE_AUTO = 0,
    WHITE_BALANCE_DAYLIGHT = 1,
    WHITE_BALANCE_CLOUDY = 2,
    WHITE_BALANCE_TUNGSTEN = 3,
    WHITE_BALANCE_FLUORESCENT = 4
};

enum {
    SUBJECT_PROGRAM_NONE = 0,
    SUBJECT_PROGRAM_PORTRAIT = 1,
    SUBJECT_PROGRAM_TEXT = 2,
    SUBJECT_PROGRAM_NIGHT_PORTRAIT = 3,
    SUBJECT_PROGRAM_SUNSET = 4,
    SUBJECT_PROGRAM_SPORTS_ACTION = 5
};

enum {
    COLOR_MODE_NORMAL = 0,
    COLOR_MODE_BLACK_AND_WHITE = 1,
    COLOR_MODE_VIVID_COLOR = 2,  /* D7i, D7Hi */
    COLOR_MODE_SOLARIZATION = 3, /* D7i, D7Hi */
    COLOR_MODE_ADOBE_RGB = 4     /* D7Hi */
};

/* Known tags found in the main IFD directory.
 */
enum {
    IFDTAG_WIDTH = 0x0100,      /* Image width. */
    IFDTAG_HEIGHT = 0x0101,     /* Image height. */
    IFDTAG_COMPRESS = 0x0103,   /* Compression. */
    IFDTAG_DCFVER = 0x010E,     /* DCF version (string). */
    IFDTAG_MANUF = 0x010F,      /* Manufacturer (string). */
    IFDTAG_CAMERA = 0x0110,     /* Camera name (string). */
    IFDTAG_FIRMWARE = 0x0131,   /* Firmware version (string). */
    IFDTAG_DATETIME = 0x0132,   /* Date time (string). */
    IFDTAG_EXIFOFFSET = 0x8769, /* Offset of EXIF data (long). */
    IFDTAG_PIMOFFSET = 0xC4A5   /* Offset of PIM info (some cameras only). */
};

/* Known tags found in the Manufacturer's directory. */
enum {
    MRWTAG_THUMBNAIL =
        0x0081, /* Offset to Thumbnail data (early cameras only). */
    MRWTAG_THUMBNAIL_OFFSET = 0x0088,
    MRWTAG_THUMBNAIL_LENGTH = 0x0089
};
}

/** A container for a Minolta Raw object.
 */
class MRWContainer : public IfdFileContainer {
public:
    MRWContainer(const IO::Stream::Ptr &file, off_t offset = 0);
    /** destructor */
    virtual ~MRWContainer();

    MRWContainer(const MRWContainer &) = delete;
    MRWContainer &operator=(const MRWContainer &) = delete;

    /**
     * Check the MRW magic header.
     */
    virtual IfdFileContainer::EndianType isMagicHeader(const char *p, int len) override;

    /* Known datablocks within an MRW file.
     */
    MRW::DataBlock::Ref mrm;
    MRW::DataBlock::Ref prd;
    MRW::DataBlock::Ref ttw;
    MRW::DataBlock::Ref wbg;
    MRW::DataBlock::Ref rif;

    /** Return offset of pixel array data from start of file.
     */
    off_t pixelDataOffset() {
        /* The pixel data immediately follows the MRM datablock. */
        return mrm->offset() + MRW::DataBlockHeaderLength + mrm->length();
    }

protected:
    virtual bool locateDirsPreHook() override;

private:
    std::string m_version;

};
}
}

#endif
