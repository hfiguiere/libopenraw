/*
 * libopenraw - ifdentry.cpp
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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


#include <cassert>
#include <string>

#include "exception.h"
#include "endianutils.h"

#include "trace.h"
#include "ifdfilecontainer.h"
#include "ifdentry.h"
#include "ifd.h"

namespace OpenRaw {
namespace Internals {


IfdEntry::IfdEntry(uint16_t _id, int16_t _type, 
									 int32_t _count, uint32_t _data,
									 IfdFileContainer &_container)
	: m_id(_id), m_type(_type),				
	  m_count(_count), m_data(_data), 
	  m_loaded(false), m_dataptr(NULL), 
	  m_container(_container)
{
}


IfdEntry::~IfdEntry()
{
	if (m_dataptr) {
		free(m_dataptr);
	}
}

RawContainer::EndianType IfdEntry::endian() const
{
	return m_container.endian();
}


bool IfdEntry::loadData(size_t unit_size)
{
	bool success = false;
	size_t data_size = unit_size * m_count;
	if (data_size <= 4) {
		m_dataptr = NULL;
		success = true;
	}
	else {
		off_t _offset;
		if (endian() == RawContainer::ENDIAN_LITTLE) {
			_offset = IfdTypeTrait<uint32_t>::EL((uint8_t*)&m_data);
		}
		else {
			_offset = IfdTypeTrait<uint32_t>::BE((uint8_t*)&m_data);
		}
		m_dataptr = (uint8_t*)realloc(m_dataptr, data_size);
		success = (m_container.fetchData(m_dataptr, 
										 _offset, 
										 data_size) == data_size);
	}
	return success;
}

uint32_t IfdEntry::getIntegerArrayItem(int idx)
{
    uint32_t v = 0;
    
    try {
        switch(type())
        {
        case IFD::EXIF_FORMAT_LONG:
            v = IfdTypeTrait<uint32_t>::get(*this, idx);
            break;
        case IFD::EXIF_FORMAT_SHORT:
            v = IfdTypeTrait<uint16_t>::get(*this, idx);
            break;
        case IFD::EXIF_FORMAT_RATIONAL:
        {
            IFD::Rational r = IfdTypeTrait<IFD::Rational>::get(*this, idx);
            if(r.denom == 0) {
                v = 0;
            }
            else {
                v = r.num / r.denom;
            }
            break;
        }
        default:
            break;
        }
    }
    catch(const std::exception & ex) {
        Debug::Trace(ERROR) << "Exception raised " << ex.what() 
                     << " fetch integer value for " << m_id << "\n";
    }

    return v;
}

template <>
const uint16_t IfdTypeTrait<uint8_t>::type = IFD::EXIF_FORMAT_BYTE;
template <>
const size_t IfdTypeTrait<uint8_t>::size = 1;

template <>
const uint16_t IfdTypeTrait<uint16_t>::type = IFD::EXIF_FORMAT_SHORT;
template <>
const size_t IfdTypeTrait<uint16_t>::size = 2;

template <>
const uint16_t IfdTypeTrait<IFD::Rational>::type = IFD::EXIF_FORMAT_RATIONAL;
template <>
const size_t IfdTypeTrait<IFD::Rational>::size = 8;


#if defined(__APPLE_CC__)
// Apple broken g++ version or linker seems to choke.
template <>
const uint16_t IfdTypeTrait<unsigned long>::type = IFD::EXIF_FORMAT_LONG;
template <>
const size_t IfdTypeTrait<unsigned long>::size = 4;
#endif
template <>
const uint16_t IfdTypeTrait<uint32_t>::type = IFD::EXIF_FORMAT_LONG;
template <>
const size_t IfdTypeTrait<uint32_t>::size = 4;

template <>
const uint16_t IfdTypeTrait<std::string>::type = IFD::EXIF_FORMAT_ASCII;
template <>
const size_t IfdTypeTrait<std::string>::size = 1;
}
}
