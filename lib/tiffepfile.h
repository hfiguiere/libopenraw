/*
 * libopenraw - tiffepfile.h
 *
 * Copyright (C) 2007-2015 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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


#ifndef OR_INTERNALS_TIFF_EP_FILE_H_
#define OR_INTERNALS_TIFF_EP_FILE_H_

#include <libopenraw++/rawfile.h>

#include "ifddir.h"
#include "ifdfile.h"
#include "io/stream.h"

namespace OpenRaw {
namespace Internals {


/** This is for TIFF EP conformant files. This include DNG, NEF, 
 *  ERF */
class TiffEpFile
    : public IfdFile
{
public:
    TiffEpFile(const IO::Stream::Ptr &s, Type _type);

protected:

    virtual IfdDir::Ref  _locateCfaIfd();
    virtual IfdDir::Ref  _locateMainIfd();
};

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
#endif
