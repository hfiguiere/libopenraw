/*
 * libopenraw - makernotedir.h
 *
 * Copyright (C) Hubert Figuiere
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


#ifndef __OPENRAW_INTERNALS_MAKERNOTEDIR_H__
#define __OPENRAW_INTERNALS_MAKERNOTEDIR_H__

#include "ifddir.h"

namespace OpenRaw {
namespace Internals {

class MakerNoteDir
: public IfdDir
{
public:
	typedef boost::shared_ptr<MakerNoteDir> Ref;
	
	MakerNoteDir(off_t _offset, IfdFileContainer & _container, off_t mnote_offset);

	off_t getMnoteOffset() const
		{ return m_mnote_offset; }
private:
	off_t m_mnote_offset;
};
	
	
}
}


#endif
