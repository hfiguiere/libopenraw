/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - cr3file.cpp
 *
 * Copyright (C) 2018 Hubert Figuière
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
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const RawFile::camera_ids_t Cr3File::s_def[] = {
    { "Canon EOS M50", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50) },
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
    LOGDBG1("Unimplemented\n");
    return OR_ERROR_NOT_FOUND;
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
        if (!track || (*track).track_type != MP4PARSE_TRACK_TYPE_VIDEO ||
            (*track).codec != MP4PARSE_CODEC_CRAW) {
            LOGDBG1("%u Not a CRAW track\n", i);
            continue;
        }
        auto raw_track = m_container->get_raw_track(i);
        if (!raw_track || !(*raw_track).is_jpeg) {
            LOGDBG1("%u not a video track\n", i);
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

MetaValue *Cr3File::_getMetaValue(int32_t meta_index)
{
    MetaValue *val = nullptr;
    return val;
}

void Cr3File::_identifyId()
{
    auto ifd = m_container->get_metadata_block(0);
    if (!ifd) {
        LOGERR("cr3: can't find meta block 0\n");
        return;
    }
    auto dirs = ifd->directories();
    if (dirs.empty()) {
        LOGERR("cr3: meta block 0 has no IFD\n");
        return;
    }
    const auto& mainIfd = dirs[0];
    if (!mainIfd) {
        LOGERR("cr3: IFD 0 isn't available\n");
        return;
    }
    mainIfd->load();
    auto make = mainIfd->getValue<std::string>(IFD::EXIF_TAG_MAKE);
    auto model = mainIfd->getValue<std::string>(IFD::EXIF_TAG_MODEL);
    if (make && model) {
        _setTypeId(_typeIdFromModel(make.value(), model.value()));
    } else {
        LOGERR("make or model not found\n");
    }
}

}
}
