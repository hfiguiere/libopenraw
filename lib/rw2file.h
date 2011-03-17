/*
 * libopenraw - rw2file.h
 *
 * Copyright (C) 2006, 2008 Hubert Figuiere
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




#ifndef __RW2FILE_H_
#define __RW2FILE_H_

#include "ifdfile.h"
#include "rawfilefactory.h"
#include "ifdfilecontainer.h"

namespace OpenRaw {

class Thumbnail;
	
namespace Internals {
		
class Rw2File
	: public IfdFile
{
public:
	static RawFile *factory(IO::Stream * s);
	Rw2File(IO::Stream *s);
	virtual ~Rw2File();

protected:
	virtual IfdDir::Ref  _locateCfaIfd();
	virtual IfdDir::Ref  _locateMainIfd();

private:
	
	Rw2File(const Rw2File&);
	Rw2File & operator=(const Rw2File&);

	virtual ::or_error _locateThumbnail(const IfdDir::Ref & dir,
                                     std::vector<uint32_t> &list);
	virtual uint32_t _getJpegThumbnailOffset(const IfdDir::Ref & dir, uint32_t & len);
	virtual ::or_error _getRawData(RawData & data, uint32_t options);

	static const IfdFile::camera_ids_t s_def[];
};

}
}

#endif
