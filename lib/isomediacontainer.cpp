/* -*- tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - isomediacontainer.cpp
 *
 * Copyright (C) 2018-2020 Hubert Figuiere
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

#include "isomediacontainer.hpp"
#include "trace.hpp"
#include "io/memstream.hpp"

namespace OpenRaw {
namespace Internals {


IsoMediaContainer::IsoMediaContainer(const IO::Stream::Ptr &file)
    : RawContainer(file, 0)
    , m_mp4io{ &IsoMediaContainer::read_callback, static_cast<void*>(file.get()) }
    , m_parser(nullptr)
{
    setEndian(ENDIAN_BIG);
    m_file->seek(0, SEEK_SET);
    auto status = mp4parse_new(&m_mp4io, &m_parser);
    if (status != MP4PARSE_STATUS_OK) {
        LOGERR("IsoM: failed to create parser: %d\n", status);
    }
}

IsoMediaContainer::~IsoMediaContainer()
{
    if (m_parser) {
        mp4parse_free(m_parser);
    }
}

uint32_t IsoMediaContainer::count_tracks()
{
    uint32_t count = 0;
    auto status = mp4parse_get_track_count(m_parser, &count);
    if (status != MP4PARSE_STATUS_OK) {
        LOGERR("IsoM: get_track_count() failed %d\n", status);
        return 0;
    }

    return count;
}

Option<Mp4parseTrackInfo>
IsoMediaContainer::get_track(uint32_t index)
{
    Mp4parseTrackInfo info;
    auto status = mp4parse_get_track_info(m_parser, index, &info);
    if (status != MP4PARSE_STATUS_OK) {
        return OptionNone();
    }
    return option_some(std::move(info));
}

Option<Mp4parseTrackRawInfo>
IsoMediaContainer::get_raw_track(uint32_t index)
{
    Mp4parseTrackRawInfo info;
    auto status = mp4parse_get_track_raw_info(m_parser, index, &info);
    if (status != MP4PARSE_STATUS_OK) {
        return OptionNone();
    }
    return option_some(std::move(info));
}

Option<Mp4parseCrawHeader>
IsoMediaContainer::get_craw_header()
{
    Mp4parseCrawHeader header;
    auto status = mp4parse_get_craw_header(m_parser, &header);
    if (status != MP4PARSE_STATUS_OK) {
        return OptionNone();
    }
    return option_some(std::move(header));
}

Option<std::pair<uint64_t, uint64_t>>
IsoMediaContainer::get_offsets_at(uint32_t index)
{
    std::pair<uint64_t, uint64_t> entry;
    auto status = mp4parse_get_craw_table_entry(m_parser, index,
                                                &entry.first, &entry.second);
    if (status != MP4PARSE_STATUS_OK) {
        return OptionNone();
    }
    return option_some(std::move(entry));
}

Option<ThumbDesc>
IsoMediaContainer::get_preview_desc()
{
    auto preview_offset = get_offsets_at(1);
    if (preview_offset) {
        auto offset = (*preview_offset).first;
        // box (24) + content (8) + prvw box (8) + unknown (4)
        // We need to skip the "boxes" (ISO container)
        // And skip a short (16bits) value.
        offset += 44 + 2;
        m_file->seek(offset, SEEK_SET);
        auto width = readUInt16(m_file, m_endian);
        auto height = readUInt16(m_file, m_endian);
        skip(2);
        auto jpeg_size = readUInt32(m_file, m_endian);
        if (width && height && jpeg_size) {
            return option_some(
                std::move(ThumbDesc(*width, *height, OR_DATA_TYPE_JPEG,
                                    offset + 10, *jpeg_size)));
        }
    }
    return OptionNone();
}

std::shared_ptr<IfdFileContainer>
IsoMediaContainer::get_metadata_block(uint32_t idx)
{
    if (m_meta_ifd.empty()) {
        auto craw = get_craw_header();
        if (!craw) {
            return std::shared_ptr<IfdFileContainer>();
        }

        m_meta_ifd.resize(4);
        if ((*craw).meta1.length) {
            auto mem = std::make_shared<IO::MemStream>(
                (*craw).meta1.data, (*craw).meta1.length);
            m_meta_ifd[0] = std::make_shared<IfdFileContainer>(mem, 0);
        } else {
            m_meta_ifd[0] = std::shared_ptr<IfdFileContainer>();
        }
        if ((*craw).meta2.length) {
            auto mem = std::make_shared<IO::MemStream>(
                (*craw).meta2.data, (*craw).meta2.length);
            m_meta_ifd[1] = std::make_shared<IfdFileContainer>(mem, 0);
        } else {
            m_meta_ifd[1] = std::shared_ptr<IfdFileContainer>();
        }
        if ((*craw).meta3.length) {
            auto mem = std::make_shared<IO::MemStream>(
                (*craw).meta3.data, (*craw).meta3.length);
            m_meta_ifd[2] = std::make_shared<IfdFileContainer>(mem, 0);
        } else {
            m_meta_ifd[2] = std::shared_ptr<IfdFileContainer>();
        }
        if ((*craw).meta4.length) {
            auto mem = std::make_shared<IO::MemStream>(
                (*craw).meta4.data, (*craw).meta4.length);
            m_meta_ifd[3] = std::make_shared<IfdFileContainer>(mem, 0);
        } else {
            m_meta_ifd[3] = std::shared_ptr<IfdFileContainer>();
        }
    }
    if (idx < m_meta_ifd.size()) {
        return m_meta_ifd.at(idx);
    }
    return std::shared_ptr<IfdFileContainer>();
}

intptr_t
IsoMediaContainer::read_callback(uint8_t* buf, uintptr_t len, void* self)
{
    IO::Stream* stream = static_cast<IO::Stream*>(self);
    return stream->read(buf, len);
}

}
}
