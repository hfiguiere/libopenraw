/*
 * libopenraw - dngfile.h
 *
 * Copyright (C) 2006-2014 Hubert Figuiere
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

#include "tiffepfile.h"

namespace OpenRaw {

class Thumbnail;

namespace Internals {
class IOFile;
class IFDFileContainer;

class DngFile
    : public TiffEpFile
{
public:
    static RawFile *factory(const IO::Stream::Ptr &);

    DngFile(const IO::Stream::Ptr &);
    virtual ~DngFile();

    /** DNG specific for now: check if file is Cinema DNG. */
    bool isCinema() const;
protected:
    virtual ::or_error _getRawData(RawData & data, uint32_t options);
    virtual void _identifyId();

private:

    DngFile(const DngFile&) = delete;
    DngFile & operator=(const DngFile&) = delete;

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
