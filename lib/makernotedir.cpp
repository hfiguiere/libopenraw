/*
 * libopenraw - makernotedir.cpp
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


#include "makernotedir.h"


namespace OpenRaw {
namespace Internals {

MakerNoteDir::MakerNoteDir(off_t _offset, IfdFileContainer & _container, off_t mnote_offset)
	: IfdDir(_offset, _container)
	, m_mnote_offset(mnote_offset)
{
}


}
}
