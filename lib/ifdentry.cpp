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

#include "exception.h"
#include "endianutils.h"

#include "ifdfilecontainer.h"
#include "ifdentry.h"
#include "ifd.h"

namespace OpenRaw {
	namespace Internals {


		IFDEntry::IFDEntry(int16_t _id, int16_t _type, 
											 int32_t _count, uint32_t _data,
											 IFDFileContainer &_container)
			: m_id(_id), m_type(_type),				
				m_count(_count), m_data(_data), 
				m_container(_container)
		{
		}


		IFDEntry::~IFDEntry()
		{
		}

		uint32_t IFDEntry::getLong() 
			throw (BadTypeException, TooBigException)
		{
			if (m_type != IFD::EXIF_FORMAT_LONG) {
				throw BadTypeException();
			}
			if (m_count > 1) {
				throw TooBigException();
			}
			uint32_t val;
			if (m_container.endian() == RawContainer::ENDIAN_LITTLE) {
				val = EL32((uint8_t*)&m_data);
			}
			else {
				val = BE32((uint8_t*)&m_data);
			}
			return val;
		}

		uint16_t IFDEntry::getShort() 
			throw (BadTypeException, TooBigException)
		{
			if (m_type != IFD::EXIF_FORMAT_SHORT) {
				throw BadTypeException();
			}
			if (m_count > 1) {
				throw TooBigException();
			}
			uint32_t val;
			if (m_container.endian() == RawContainer::ENDIAN_LITTLE) {
				val = EL16((uint8_t*)&m_data);
			}
			else {
				val = BE16((uint8_t*)&m_data);
			}
			return val;
		}

	}
}
