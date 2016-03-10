/*
 * libopenraw - erffile.cpp
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


#include <libopenraw/cameraids.h>

#include "ifddir.hpp"
#include "rawfile_private.hpp"
#include "erffile.hpp"

using namespace Debug;

namespace OpenRaw {

class RawData;

namespace Internals {

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON, OR_TYPEID_EPSON_RD1), 0, 0,
      { 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON, OR_TYPEID_EPSON_RD1S), 0, 0,
      { 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t ERFFile::s_def[] = {
    { "R-D1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON,
                                  OR_TYPEID_EPSON_RD1) },
    { "R-D1s", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON,
                                   OR_TYPEID_EPSON_RD1S) },			{ 0, 0 }
};

RawFile *ERFFile::factory(const IO::Stream::Ptr &s)
{
    return new ERFFile(s);
}

ERFFile::ERFFile(const IO::Stream::Ptr &s)
    : TiffEpFile(s, OR_RAWFILE_TYPE_ERF)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

ERFFile::~ERFFile()
{
}

::or_error ERFFile::_getRawData(RawData & data, uint32_t /*options*/)
{
    ::or_error err;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    if(_cfaIfd) {
        err = _getRawDataFromDir(data, _cfaIfd);
    }
    else {
        err = OR_ERROR_NOT_FOUND;
    }
    return err;
}

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
