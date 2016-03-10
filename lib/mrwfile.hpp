/*
 * libopenraw - mrwfile.h
 *
 * Copyright (C) 2006-2016 Hubert Figuière
 * Copyright (C) 2008 Bradley Broom
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

#ifndef OR_INTERNALS_MRWFILE_H_
#define OR_INTERNALS_MRWFILE_H_

#include <stdint.h>
#include <vector>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"

namespace OpenRaw {

class RawData;
class Thumbnail;

namespace Internals {

class MRWFile
    : public IfdFile
{
public:
    static RawFile *factory(const IO::Stream::Ptr&);
    MRWFile(const IO::Stream::Ptr &);
    virtual ~MRWFile();

    MRWFile(const MRWFile&) = delete;
    MRWFile & operator=(const MRWFile&) = delete;

protected:
    virtual IfdDir::Ref  _locateCfaIfd() override;
    virtual IfdDir::Ref  _locateMainIfd() override;

    virtual void _identifyId() override;

    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;
    virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail) override;
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
private:

    static const struct IfdFile::camera_ids_t s_def[];
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
