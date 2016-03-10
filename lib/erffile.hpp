/* -*- Mode: C++ -*- */
/*
 * libopenraw - erffile.h
 *
 * Copyright (C) 2007-2016 Hubert Figuiere
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

#ifndef OR_INTERNALS_ERFFILE_H_
#define OR_INTERNALS_ERFFILE_H_

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
public:
    static RawFile *factory(const IO::Stream::Ptr &);
    ERFFile(const IO::Stream::Ptr &);
    virtual ~ERFFile();

    ERFFile(const ERFFile&) = delete;
    ERFFile & operator=(const ERFFile &) = delete;

protected:
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
private:

    static const IfdFile::camera_ids_t s_def[];
};

}
}

#endif
