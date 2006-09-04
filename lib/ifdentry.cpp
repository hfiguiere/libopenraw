/*
 * libopenraw - ifdentry.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <cassert>

#include "ifdfilecontainer.h"
#include "ifdentry.h"

namespace OpenRaw {
	namespace Internals {


		IFDEntry::IFDEntry(Int16 _id, Int16 _type, 
											 Int32 _count, Int32 _offset,
											 IFDFileContainer &_container)
			: m_id(_id), m_type(_type),
				m_count(_count), m_offset(_offset), 
				m_container(_container)
		{
		}


		IFDEntry::~IFDEntry()
		{
		}

		Int32 IFDEntry::getLong()
		{
			assert(m_type == 4);
			assert(m_count == 1);
			return m_offset;
		}

	}
}
