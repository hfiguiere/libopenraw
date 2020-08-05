/* -*- Mode: C++ -*- */
/*
 * libopenraw - erffile.hpp
 *
 * Copyright (C) 2007-2020 Hubert Figuière
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

#pragma

#include <stdint.h>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"
#include "tiffepfile.hpp"

namespace OpenRaw {

namespace Internals {

class ERFFile
    : public TiffEpFile
{
    template<typename T>
    friend void audit_coefficients();

public:
    static RawFile *factory(const IO::Stream::Ptr &);
    ERFFile(const IO::Stream::Ptr &);
    virtual ~ERFFile();

    ERFFile(const ERFFile&) = delete;
    ERFFile & operator=(const ERFFile &) = delete;

protected:
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t>& list) override;
    virtual ::or_error _getThumbnail(uint32_t size, Thumbnail& thumbnail) override;
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
private:
    ::or_error getMakerNoteThumbnail(Thumbnail& thumbnail);

    static const IfdFile::camera_ids_t s_def[];
};

}
}
