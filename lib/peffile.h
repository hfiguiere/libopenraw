/*
 * libopenraw - peffile.h
 *
 * Copyright (C) 2006-2015 Hubert Figuiere
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

#ifndef OR_INTERNALS_PEFFILE_H_
#define OR_INTERNALS_PEFFILE_H_

#include <stdint.h>

#include <libopenraw/consts.h>
#include <libopenraw++/rawfile.h>

#include "ifddir.h"
#include "io/stream.h"
#include "ifdfile.h"

namespace OpenRaw {

class RawData;

namespace Internals {

class PEFFile
    : public IfdFile
{
public:
    static RawFile *factory(const IO::Stream::Ptr &s);
    PEFFile(const IO::Stream::Ptr &);
    virtual ~PEFFile();
    
protected:
    virtual IfdDir::Ref  _locateCfaIfd();
    virtual IfdDir::Ref  _locateMainIfd();
    
    virtual ::or_error _getRawData(RawData & data, uint32_t options);
private:
    
    PEFFile(const PEFFile&) = delete;
    PEFFile & operator=(const PEFFile &) = delete;
    
    static const IfdFile::camera_ids_t s_def[];
};

}
}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
