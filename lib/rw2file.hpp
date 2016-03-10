/* -*- Mode: C++ -*- */
/*
 * libopenraw - rw2file.h
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

#ifndef OR_INTERNALS_RW2FILE_H_
#define OR_INTERNALS_RW2FILE_H_

#include <stdint.h>
#include <vector>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifddir.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"

namespace OpenRaw {

class RawData;
	
namespace Internals {
		
class Rw2File
	: public IfdFile
{
public:
	static RawFile *factory(const IO::Stream::Ptr & s);
	Rw2File(const IO::Stream::Ptr & s);
	virtual ~Rw2File();

  Rw2File(const Rw2File&) = delete;
	Rw2File & operator=(const Rw2File&) = delete;

	enum {
		PANA_RAW_COMPRESSION = 0x11000
	};

protected:
	virtual IfdDir::Ref  _locateCfaIfd() override;
	virtual IfdDir::Ref  _locateMainIfd() override;
private:

	virtual ::or_error _locateThumbnail(const IfdDir::Ref & dir,
                                     std::vector<uint32_t> &list) override;
	virtual uint32_t _getJpegThumbnailOffset(const IfdDir::Ref & dir, uint32_t & len) override;
	virtual ::or_error _getRawData(RawData & data, uint32_t options) override;

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
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
