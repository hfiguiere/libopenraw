/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - cr3file.cpp
 *
 * Copyright (C) 2018-2020 Hubert Figuière
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

#include <cstdint>
#include <memory>
#include <stddef.h>
#include <vector>

#include <libopenraw/cameraids.h>
#include <libopenraw/consts.h>
#include <libopenraw/debug.h>

#include "cr3file.hpp"
#include "isomediacontainer.hpp"
#include "canon.hpp"
#include "rawdata.hpp"
#include "rawfile_private.hpp"
#include "bitmapdata.hpp"
#include "trace.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_CANON_TYPEID(camid)                                            \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, camid)

/* all relative to the D65 calibration illuminant */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M200),
      0,
      0,
      { 10463, -2173, -1437, -4856, 12635, 2482, -1216, 2915, 7237 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M6MKII),
      0,
      0,
      { 11498, -3759, -1516, -5073, 12954, 2349, -892, 1867, 6118 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_90D),
      0,
      0,
      { 11498, -3759, -1516, -5073, 12954, 2349, -892, 1867, 6118 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R),
      0,
      0,
      { 6446, -366, -864, -4436, 12204, 2513, -952, 2496, 6348 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_RP),
      0,
      0,
      { 8608, -2097, -1178, -5425, 13265, 2383, -1149, 2238, 5680 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R5),
      0,
      0,
      { 9766, -2953, -1254, -4276, 12116, 2433, -437, 1336, 5131 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R6),
      0,
      0,
      { 8293, -1611, -1132, -4759, 12711, 2275, -1013, 2415, 5509 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX70_HS),
      0,
      0,
      { 18285, -8907, -1951, -1845, 10688, 1323, 364, 1101, 5139 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_250D),
      0,
      0,
      { 9079, -1923, -1236, -4677, 12454, 2492, -922, 2319, 5565 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5XMKII),
      0,
      0,
      { 11629, -5713, -914, -2706, 11090, 1842, -206, 1225, 5515 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7XMKIII),
      0,
      0,
      { 11629, -5713, -914, -2706, 11090, 1842, -206, 1225, 5515 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DXMKIII),
      0,
      0,
      { 8971, -2022, -1242, -5405, 13249, 2380, -1280, 2483, 6072 } },
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const RawFile::camera_ids_t Cr3File::s_def[] = {
    { "Canon EOS M50", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50) },
    { "Canon EOS M200", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M200) },
    { "Canon EOS R", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R) },
    { "Canon EOS RP", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_RP) },
    { "Canon EOS R5", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R5) },
    { "Canon EOS R6", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R6) },
    { "Canon EOS 250D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_250D) },
    { "Canon EOS Rebel SL3", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_250D) },
    { "Canon PowerShot SX70 HS", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX70_HS) },
    { "Canon PowerShot G5 X Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5XMKII) },
    { "Canon PowerShot G7 X Mark III", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7XMKIII) },
    { "Canon EOS-1D X Mark III", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DXMKIII) },
    { "Canon EOS M6 Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M6MKII) },
    { "Canon EOS 90D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_90D) },
    { 0, 0 }
};

RawFile *Cr3File::factory(const IO::Stream::Ptr &s)
{
    return new Cr3File(s);
}

Cr3File::Cr3File(const IO::Stream::Ptr &s)
    : RawFile(OR_RAWFILE_TYPE_CR3)
    , m_io(s)
    , m_container(new IsoMediaContainer(s))
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

Cr3File::~Cr3File()
{
    delete m_container;
}

RawContainer *Cr3File::getContainer() const
{
    return m_container;
}

::or_error Cr3File::_getRawData(RawData &data, uint32_t options)
{
    auto track = m_container->get_track(2);
    if (!track || (*track).track_type != MP4PARSE_TRACK_TYPE_VIDEO) {
        LOGERR("%u Not a video track\n", 2);
        return OR_ERROR_NOT_FOUND;
    }
    auto raw_track = m_container->get_raw_track(2);
    if (!raw_track || (*raw_track).is_jpeg) {
        LOGERR("%u not the RAW data track\n", 2);
        return OR_ERROR_NOT_FOUND;
    }

    if ((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
        LOGWARN("Can't provide decompressed data yet. Ignoring.\n");
    }

    data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);
    data.setDimensions((*raw_track).image_width, (*raw_track).image_height);
    // get the sensor info
    const IfdDir::Ref &makerNoteIfd = _getMakerNoteIfd();
    auto sensorInfo = canon_get_sensorinfo(makerNoteIfd);
    if (sensorInfo) {
        data.setActiveArea((*sensorInfo)[0], (*sensorInfo)[1],
                           (*sensorInfo)[2], (*sensorInfo)[3]);
    }

    auto byte_length = (*raw_track).size;
    void* p = data.allocData(byte_length);
    size_t real_size = m_container->fetchData(p, (*raw_track).offset,
                                              byte_length);
    if (real_size < byte_length) {
        LOGWARN("Size mismatch for data: ignoring.\n");
    }
    return OR_ERROR_NONE;
}

::or_error Cr3File::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
    auto err = OR_ERROR_NOT_FOUND;
    auto craw_header = m_container->get_craw_header();
    if (craw_header) {
        uint32_t x = (*craw_header).thumb_w;
        uint32_t y = (*craw_header).thumb_h;
        auto dim = std::max(x, y);
        if (dim) {
            list.push_back(dim);
            std::unique_ptr<BitmapData> data(new BitmapData);
            data->setDimensions(x, y);
            data->setDataType(OR_DATA_TYPE_JPEG);
            void* p = data->allocData((*craw_header).thumbnail.length);
            ::memcpy(p, (*craw_header).thumbnail.data, (*craw_header).thumbnail.length);
            _addThumbnail(dim, ThumbDesc(x, y, OR_DATA_TYPE_JPEG, std::move(data)));
            err = OR_ERROR_NONE;
        }
    }
    auto track_count = m_container->count_tracks();
    for (uint32_t i = 0; i < track_count; i++) {
        auto track = m_container->get_track(i);
        if (!track || (*track).track_type != MP4PARSE_TRACK_TYPE_VIDEO) {
            LOGDBG1("%u Not a video track\n", i);
            continue;
        }
        auto raw_track = m_container->get_raw_track(i);
        if (!raw_track || !(*raw_track).is_jpeg) {
            LOGDBG1("%u not a RAW data track\n", i);
            continue;
        }
        auto dim = std::max((*raw_track).image_width,
                            (*raw_track).image_height);
        LOGDBG1("Dimension %u\n", dim);
        list.push_back(dim);
        _addThumbnail(dim, ThumbDesc((*raw_track).image_width,
                                     (*raw_track).image_height,
                                     OR_DATA_TYPE_JPEG, (*raw_track).offset,
                                     (*raw_track).size));
        err = OR_ERROR_NONE;
    }

    auto desc = m_container->get_preview_desc();
    if (desc) {
        auto dim = std::max((*desc).x, (*desc).y);
        list.push_back(dim);
        _addThumbnail(dim, desc.value());
        err = OR_ERROR_NONE;
    }

    return err;
}


IfdDir::Ref Cr3File::findIfd(uint32_t idx)
{
    if (idx >= m_ifds.size()) {
        LOGERR("Invalid ifd index %u\n", idx);
        return IfdDir::Ref();
    }

    auto ifd = m_ifds[idx];

    if (!ifd) {
        ifd = m_container->get_metadata_block(idx);
        if (!ifd) {
            LOGERR("cr3: can't find meta block 0\n");
            return IfdDir::Ref();
        }
    }

    return ifd->setDirectory(0);
}

IfdDir::Ref Cr3File::mainIfd()
{
    return findIfd(0);
}


IfdDir::Ref Cr3File::exifIfd()
{
    return findIfd(1);
}


IfdDir::Ref Cr3File::_getMakerNoteIfd()
{
    return findIfd(2);
}

MetaValue* Cr3File::_getMetaValue(int32_t meta_index)
{
    MetaValue *val = nullptr;
    IfdDir::Ref ifd;
    // I wish I had an HaveIFD "trait" for this.
    // This is almost IfdFile::_getMetaValue()
    if (META_INDEX_MASKOUT(meta_index) == META_NS_TIFF) {
        ifd = mainIfd();
    } else if (META_INDEX_MASKOUT(meta_index) == META_NS_EXIF) {
        ifd = exifIfd();
    } else {
        LOGERR("Unknown Meta Namespace\n");
    }
    if(ifd) {
        LOGDBG1("Meta value for %u\n", META_NS_MASKOUT(meta_index));

        IfdEntry::Ref e = ifd->getEntry(META_NS_MASKOUT(meta_index));
        if(e) {
            val = e->make_meta_value();
        }
    }

    return val;
}

void Cr3File::_identifyId()
{
    // There is a camera model ID in the MakerNote tag 0x0010.
    // Use this at first.
    auto mn = getMakerNoteIfd();
    if (mn) {
        auto id = mn->getValue<uint32_t>(IFD::MNOTE_CANON_MODEL_ID);
        if (id) {
            auto id_value = id.value();
            auto type_id = canon_modelid_to_typeid(id_value);
            if (type_id != 0) {
                _setTypeId(type_id);
                return;
            }
            LOGERR("model ID %x not found\n", id_value);
        } else {
            LOGERR("model ID not found\n");
        }
    } else {
        LOGERR("model ID not found (missing MakerNote)\n");
    }
}

}
}
