/*
 * libopenraw - ifdentry.cpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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


#include <stdlib.h>
#include <math.h>

#include <cstdint>
#include <string>

#include <libopenraw/debug.h>

#include "trace.hpp"
#include "ifdfilecontainer.hpp"
#include "ifdentry.hpp"
#include "ifd.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {


IfdEntry::IfdEntry(uint16_t _id, int16_t _type,
                   int32_t _count, uint32_t _data,
                   const IfdFileContainer& _container)
    : m_id(_id), m_type(_type),
      m_count(_count), m_data(_data),
      m_loaded(false), m_dataptr(NULL),
      m_container(_container)
{
	auto container_size = m_container.size();
	auto unit_size = typeUnitSize(static_cast<IFD::ExifTagType>(m_type));
	if ((m_count * unit_size) > static_cast<size_t>(container_size)) {
		LOGERR("Trying to have %u items in a container of %lld bytes\n",
			   m_count, (long long int)container_size);
		m_count = container_size / unit_size;
	}
}


IfdEntry::~IfdEntry()
{
    if (m_dataptr) {
        free(m_dataptr);
    }
}

size_t IfdEntry::typeUnitSize(IFD::ExifTagType _type)
{
	switch(_type) {
    case IFD::EXIF_FORMAT_BYTE:
    case IFD::EXIF_FORMAT_SBYTE:
    case IFD::EXIF_FORMAT_ASCII:
    case IFD::EXIF_FORMAT_UNDEFINED:
		return 1;
    case IFD::EXIF_FORMAT_SHORT:
    case IFD::EXIF_FORMAT_SSHORT:
		return 2;
    case IFD::EXIF_FORMAT_LONG:
    case IFD::EXIF_FORMAT_SLONG:
    case IFD::EXIF_FORMAT_FLOAT:
		return 4;
    case IFD::EXIF_FORMAT_RATIONAL:
    case IFD::EXIF_FORMAT_SRATIONAL:
    case IFD::EXIF_FORMAT_DOUBLE:
		return 8;
	}

	return 0;
}

RawContainer::EndianType IfdEntry::endian() const
{
	return m_container.endian();
}


bool IfdEntry::loadData(size_t unit_size, off_t offset)
{
	if (!m_loaded) {
		size_t data_size = unit_size * m_count;
		if (data_size <= 4) {
			m_dataptr = NULL;
			m_loaded = true;
		} else {
			off_t _offset;
			if (endian() == RawContainer::ENDIAN_LITTLE) {
				_offset = IfdTypeTrait<uint32_t>::EL((uint8_t*)&m_data, sizeof(uint32_t));
			} else {
				_offset = IfdTypeTrait<uint32_t>::BE((uint8_t*)&m_data, sizeof(uint32_t));
			}
			_offset += m_container.exifOffsetCorrection() + offset;
			LOGDBG1("loadData: offset %lu\n", _offset);
			m_dataptr = (uint8_t*)realloc(m_dataptr, data_size);
			m_loaded = (m_container.fetchData(m_dataptr,
											  _offset,
											  data_size) == data_size);
		}
	}
	return m_loaded;
}

namespace IFD {

Rational::operator double() const
{
	if(denom == 0) {
		return INFINITY;
	}
	return (double)num / (double)denom;
}

SRational::operator double() const
{
	if(denom == 0) {
		return INFINITY;
	}
	return (double)num / (double)denom;
}

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
const uint16_t IfdTypeTrait<int8_t>::type = IFD::EXIF_FORMAT_SBYTE;
template <>
const size_t IfdTypeTrait<int8_t>::size = 1;

template <>
const uint16_t IfdTypeTrait<int16_t>::type = IFD::EXIF_FORMAT_SSHORT;
template <>
const size_t IfdTypeTrait<int16_t>::size = 2;

template <>
const uint16_t IfdTypeTrait<IFD::Rational>::type = IFD::EXIF_FORMAT_RATIONAL;
template <>
const size_t IfdTypeTrait<IFD::Rational>::size = 8;

template <>
const uint16_t IfdTypeTrait<IFD::SRational>::type = IFD::EXIF_FORMAT_SRATIONAL;
template <>
const size_t IfdTypeTrait<IFD::SRational>::size = 8;

template <>
const uint16_t IfdTypeTrait<uint32_t>::type = IFD::EXIF_FORMAT_LONG;
template <>
const size_t IfdTypeTrait<uint32_t>::size = 4;

template <>
const uint16_t IfdTypeTrait<int32_t>::type = IFD::EXIF_FORMAT_SLONG;
template <>
const size_t IfdTypeTrait<int32_t>::size = 4;

template <>
const uint16_t IfdTypeTrait<std::string>::type = IFD::EXIF_FORMAT_ASCII;
template <>
const size_t IfdTypeTrait<std::string>::size = 1;

}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:4
  c-basic-offset:4
  indent-tabs-mode:t
  fill-column:80
  End:
*/
