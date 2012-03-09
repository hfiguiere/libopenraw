/*
 * libopenraw - arwfile.cpp
 *
 * Copyright (C) 2006,2008,2011 Hubert Figuiere
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


#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>

#include "trace.h"
#include "io/file.h"
#include "ifdfilecontainer.h"
#include "ifd.h"
#include "arwfile.h"

using namespace Debug;

namespace OpenRaw {


namespace Internals {

const IfdFile::camera_ids_t ArwFile::s_def[] = {
    { "DSLR-A100", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A100) },
    { "DSLR-A200", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A200) },
    { "DSLR-A380", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A380) },
    { "DSLR-A390", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A390) },
    { "DSLR-A550", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A550) },
    { "DSLR-A580", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A580) },
    { "DSLR-A700", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A700) },
    { "SLT-A55V", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_SLTA55) },
    { "SLT-A65V", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_SLTA65) },
    { "SLT-A77V", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_SLTA77) },
    { "NEX-3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_NEX3) },
    { "NEX-5", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_NEX5) },
    { "NEX-5N", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_NEX5N) },
    // There are pre-production files with the type NEX-C00....
    { "NEX-C3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_NEXC3) },
    { "NEX-7", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_NEX7) },
    { 0, 0 }
};


RawFile *ArwFile::factory(IO::Stream * s)
{
    return new ArwFile(s);
}

ArwFile::ArwFile(IO::Stream *s)
    : TiffEpFile(s, OR_RAWFILE_TYPE_ARW)
{
    _setIdMap(s_def);
}

ArwFile::~ArwFile()
{
}

IfdDir::Ref  ArwFile::_locateCfaIfd()
{
    if(!isA100())
    {
        return TiffEpFile::_locateCfaIfd();
    }
    return mainIfd();
}


IfdDir::Ref  ArwFile::_locateMainIfd()
{
    return m_container->setDirectory(0);
}

::or_error ArwFile::_getRawData(RawData & data, uint32_t options)
{
    if(isA100())
    {
        // TODO implement for A100
        return OR_ERROR_NOT_FOUND;
    }
    return TiffEpFile::_getRawData(data, options);
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
