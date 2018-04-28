/* -*- tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - isomediacontainer.cpp
 *
 * Copyright (C) 2018 Hubert Figuiere
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

namespace OpenRaw {
namespace Internals {


IsoMediaContainer::IsoMediaContainer(const IO::Stream::Ptr &file)
    : RawContainer(file, 0)
    , m_parsed(false)
    , m_mp4io{ &IsoMediaContainer::read_callback, static_cast<void*>(file.get()) }
    , m_parser(nullptr)
{
    m_parser = mp4parse_new(&m_mp4io);
}

IsoMediaContainer::~IsoMediaContainer()
{
    if (m_parser) {
        mp4parse_free(m_parser);
    }
}

bool IsoMediaContainer::ensure_parsed()
{
    if (!m_parsed) {
        auto status = mp4parse_read(m_parser);
        if (status == MP4PARSE_STATUS_OK) {
            m_parsed = true;
        } else {
            LOGERR("IsoM: read failed %d\n", status);
        }
    }
    return m_parsed;
}

uint32_t IsoMediaContainer::count_tracks()
{
    if (!ensure_parsed()) {
        return 0;
    }
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
    if (!ensure_parsed()) {
        return OptionNone();
    }
    Mp4parseTrackInfo info;
    auto status = mp4parse_get_track_info(m_parser, index, &info);
    if (status != MP4PARSE_STATUS_OK) {
        return OptionNone();
    }
    return option_some(std::move(info));
}

Option<Mp4parseTrackVideoInfo>
IsoMediaContainer::get_video_track(uint32_t index)
{
    if (!ensure_parsed()) {
        return OptionNone();
    }
    Mp4parseTrackVideoInfo info;
    auto status = mp4parse_get_track_video_info(m_parser, index, &info);
    if (status != MP4PARSE_STATUS_OK) {
        return OptionNone();
    }
    return option_some(std::move(info));
}

intptr_t
IsoMediaContainer::read_callback(uint8_t* buf, uintptr_t len, void* self)
{
    IO::Stream* stream = static_cast<IO::Stream*>(self);
    return stream->read(buf, len);
}

}
}
