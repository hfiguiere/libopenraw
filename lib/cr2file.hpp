/* -*- Mode: C++ -*- */
/*
 * libopenraw - cr2file.h
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

#ifndef OR_INTERNALS_CR2FILE_H_
#define OR_INTERNALS_CR2FILE_H_

#include <stdint.h>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "io/stream.hpp"
#include "ifddir.hpp"
#include "ifdfile.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class Cr2File
    : public IfdFile
{
public:
    static RawFile *factory(const IO::Stream::Ptr &s);
    Cr2File(const IO::Stream::Ptr &s);
    virtual ~Cr2File();

    Cr2File(const Cr2File&) = delete;
    Cr2File & operator=(const Cr2File&) = delete;
protected:
    virtual IfdDir::Ref  _locateCfaIfd() override;
    virtual IfdDir::Ref  _locateMainIfd() override;
private:
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;

    static const IfdFile::camera_ids_t s_def[];
};

}
}

#endif
