/*
 * libopenraw - arwfile.cpp
 *
 * Copyright (C) 2006,2008 Hubert Figuiere
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

const IfdFile::camera_ids_t ARWFile::s_def[] = {
    { "DSLR-A100", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A100) },
    { "DSLR-A200", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A200) },
    { "DSLR-A550", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A550) },
    { "DSLR-A700", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,
                                       OR_TYPEID_SONY_A700) },
    { 0, 0 }
};


RawFile *ARWFile::factory(IO::Stream * s)
{
    return new ARWFile(s);
}

ARWFile::ARWFile(IO::Stream *s)
    : IfdFile(s, OR_RAWFILE_TYPE_ARW)
{
    _setIdMap(s_def);
}

ARWFile::~ARWFile()
{
}

IfdDir::Ref  ARWFile::_locateCfaIfd()
{
    // in ARW the CFA IFD is the main IFD
    return mainIfd();
}


IfdDir::Ref  ARWFile::_locateMainIfd()
{
    return m_container->setDirectory(0);
}

::or_error ARWFile::_getRawData(RawData & /*data*/, uint32_t /*options*/) 
{ 
    return OR_ERROR_NOT_FOUND; 
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
