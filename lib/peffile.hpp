/* -*- Mode: C++ -*- */
/*
 * libopenraw - peffile.h
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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

#include <stdint.h>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifddir.hpp"
#include "io/stream.hpp"
#include "ifdfile.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class PEFFile
    : public IfdFile
{
    template<typename T>
    friend void audit_coefficients();

public:
    static RawFile *factory(const IO::Stream::Ptr &s);
    PEFFile(const IO::Stream::Ptr &);
    virtual ~PEFFile();

    PEFFile(const PEFFile&) = delete;
    PEFFile & operator=(const PEFFile &) = delete;

protected:
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
    virtual bool vendorCameraIdLocation(Internals::IfdDir::Ref& ifd, uint16_t& index,
                                        const Internals::ModelIdMap*& model_map) override;
private:
    static const IfdFile::camera_ids_t s_def[];
};

}
}
