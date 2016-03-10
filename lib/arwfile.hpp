/* -*- Mode: C++ -*- */
/*
 * libopenraw - arwfile.h
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

#ifndef OR_INTERNALS_ARWFILE_H_
#define OR_INTERNALS_ARWFILE_H_

#include <libopenraw/cameraids.h>
#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "io/stream.hpp"
#include "ifddir.hpp"
#include "ifdfile.hpp"
#include "tiffepfile.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class ArwFile
    : public TiffEpFile
{
public:
    static RawFile *factory(const IO::Stream::Ptr & s);
    ArwFile(const IO::Stream::Ptr & s);
    virtual ~ArwFile();

    ArwFile(const ArwFile&) = delete;
    ArwFile & operator=(const ArwFile&) = delete;

    // this is the value for "compression" for ARW
    enum {
        ARW_RAW_COMPRESSION = 32767
    };

protected:
    virtual IfdDir::Ref  _locateCfaIfd() override;
    virtual IfdDir::Ref  _locateMainIfd() override;

    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
private:
    // first version of ARW. Different from the rest.
    bool isA100()
        {
            return typeId() == OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                                   OR_TYPEID_SONY_A100);
        }

    static const IfdFile::camera_ids_t s_def[];
};

}
}

#endif
