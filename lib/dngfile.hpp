/* -*- Mode: C++ -*- */
/*
 * libopenraw - dngfile.h
 *
 * Copyright (C) 2006-2016 Hubert Figuiere
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

#ifndef OR_INTERNALS_DNGFILE_H_
#define OR_INTERNALS_DNGFILE_H_

#include <stdint.h>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"
#include "tiffepfile.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class DngFile
    : public TiffEpFile
{
public:
    static RawFile *factory(const IO::Stream::Ptr &);

    DngFile(const IO::Stream::Ptr &);
    virtual ~DngFile();

    DngFile(const DngFile&) = delete;
    DngFile & operator=(const DngFile&) = delete;


    /** DNG specific for now: check if file is Cinema DNG. */
    bool isCinema() const;
protected:
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
    virtual void _identifyId() override;

private:

    static const IfdFile::camera_ids_t s_def[];
};

}
}

#endif
