/* -*- tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - isomediacontainer.hpp
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

#pragma once

#include "io/stream.hpp"
#include "rawcontainer.hpp"
#include "option.hpp"
#include "mp4/mp4parse.h"

namespace OpenRaw {
namespace Internals {

class IsoMediaContainer : public RawContainer {
public:
    IsoMediaContainer(const IO::Stream::Ptr &file);
    virtual ~IsoMediaContainer();

    /// Count tracks in the iso container.
    uint32_t count_tracks();
    /// Get track info.
    Option<Mp4parseTrackInfo> get_track(uint32_t index);
    Option<Mp4parseTrackRawInfo> get_raw_track(uint32_t index);
private:
    /// Ensure the mp4 is parsed.
    /// @return true if it was.
    bool ensure_parsed();

    /// Read callback for mp4 parse.
    static intptr_t read_callback(uint8_t*, uintptr_t, void*);

    bool m_parsed;
    Mp4parseIo m_mp4io;
    Mp4parseParser *m_parser;
};
}
}
