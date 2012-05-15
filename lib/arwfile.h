/*
 * libopenraw - arwfile.h
 *
 * Copyright (C) 2006-2008, 2012 Hubert Figuiere
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

#ifndef __ARWFILE_H_
#define __ARWFILE_H_

#include <libopenraw/cameraids.h>

#include "ifdfile.h"
#include "tiffepfile.h"

namespace OpenRaw {

class Thumbnail;

namespace Internals {
class IOFile;
class IfdFileContainer;

class ArwFile
    : public TiffEpFile
{
public:
    static RawFile *factory(IO::Stream* s);
    ArwFile(IO::Stream * s);
    virtual ~ArwFile();

    // this is the value for "compression" for ARW
    enum {
        ARW2_RAW_COMPRESSION = 32767
    };
    
protected:
    virtual IfdDir::Ref  _locateCfaIfd();
    virtual IfdDir::Ref  _locateMainIfd();
    
    virtual ::or_error _getRawData(RawData & data, uint32_t options);
private:
    
    ArwFile(const ArwFile&);
    ArwFile & operator=(const ArwFile&);

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
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
