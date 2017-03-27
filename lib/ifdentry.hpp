/*
 * libopenraw - ifdentry.hpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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


#ifndef OR_INTERNALS_IFDENTRY_H
#define OR_INTERNALS_IFDENTRY_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include <exception>
#include <string>
#include <vector>
#include <memory>

#include "exception.hpp"
#include "trace.hpp"
#include "endianutils.hpp"
#include "rawcontainer.hpp"
#include "ifd.hpp"

namespace OpenRaw {

class MetaValue;

namespace Internals {

class IfdFileContainer;

class IfdEntry;

/** Describe and IFDType */
template <typename T>
struct IfdTypeTrait
{
	static const uint16_t type; /**< the EXIF enum for the type */
	static const size_t   size; /**< the storage size unit in IFD*/
	static T EL(const uint8_t* d, size_t len) noexcept;
	static T BE(const uint8_t* d, size_t len) noexcept;
	static T get(IfdEntry & e, uint32_t idx = 0, bool ignore_type = false)
		noexcept(false);
};


template <>
inline uint8_t IfdTypeTrait<uint8_t>::EL(const uint8_t* b, size_t) noexcept
{
	return *b;
}

template <>
inline uint8_t IfdTypeTrait<uint8_t>::BE(const uint8_t* b, size_t) noexcept
{
	return *b;
}


template <>
inline uint16_t IfdTypeTrait<uint16_t>::EL(const uint8_t* b, size_t) noexcept
{
	return EL16(b);
}

template <>
inline uint16_t IfdTypeTrait<uint16_t>::BE(const uint8_t* b, size_t) noexcept
{
	return BE16(b);
}

template <>
inline uint32_t IfdTypeTrait<uint32_t>::EL(const uint8_t* b, size_t) noexcept
{
	return EL32(b);
}

template <>
inline uint32_t IfdTypeTrait<uint32_t>::BE(const uint8_t* b, size_t) noexcept
{
	return BE32(b);
}

template <>
inline std::string IfdTypeTrait<std::string>::EL(const uint8_t* b, size_t len) noexcept
{
  std::string s;
  try {
    s.assign((const char*)b, strnlen((const char*)b, len));
  }
  catch(...) {
  }
  return s;
}

template <>
inline std::string IfdTypeTrait<std::string>::BE(const uint8_t* b, size_t len) noexcept
{
  std::string s;
  try {
    s.assign((const char*)b, strnlen((const char*)b, len));
  }
  catch(...) {
  }
  return s;
}

template <>
inline IFD::Rational IfdTypeTrait<IFD::Rational>::EL(const uint8_t* b, size_t) noexcept
{
    IFD::Rational r;
    r.num = EL32(b);
    r.denom = EL32(b + 4);
    return r;
}

template <>
inline IFD::Rational IfdTypeTrait<IFD::Rational>::BE(const uint8_t* b, size_t) noexcept
{
    IFD::Rational r;
    r.num = BE32(b);
    r.denom = BE32(b + 4);
    return r;
}

template <>
inline IFD::SRational IfdTypeTrait<IFD::SRational>::EL(const uint8_t* b, size_t) noexcept
{
    IFD::SRational r;
    r.num = EL32(b);
    r.denom = EL32(b + 4);
    return r;
}

template <>
inline IFD::SRational IfdTypeTrait<IFD::SRational>::BE(const uint8_t* b, size_t) noexcept
{
    IFD::SRational r;
    r.num = BE32(b);
    r.denom = BE32(b + 4);
    return r;
}

class IfdEntry
{
public:
	/** Ref (ie shared pointer) */
	typedef std::shared_ptr<IfdEntry> Ref;

	IfdEntry(uint16_t _id, int16_t _type, int32_t _count,
			 uint32_t _data,
			 IfdFileContainer &_container);
	virtual ~IfdEntry();

	int16_t type() const noexcept
		{
			return m_type;
		}

	/** the count of items in the entry */
	uint32_t count() const noexcept
		{
			return m_count;
		}

	/** the offset of the data. It can just be the value
	 * if the entry is self contained.
	 */
	off_t offset() noexcept
		{
			if (endian() == RawContainer::ENDIAN_LITTLE) {
				return IfdTypeTrait<uint32_t>::EL((uint8_t*)&m_data, sizeof(uint32_t));
			}
			return IfdTypeTrait<uint32_t>::BE((uint8_t*)&m_data, sizeof(uint32_t));
		}

	RawContainer::EndianType endian() const;

public:
  MetaValue* make_meta_value();
	/**
	 * Unit size for type
	 */
	static size_t type_unit_size(IFD::ExifTagType _type);
	/** load the data for the entry
	 * if all the data fits in m_data, it is a noop
	 * @param unit_size the size of 1 unit of data
	 * @return true if success.
	 */
	bool loadData(size_t unit_size);


	/** get the array values of type T
	 * @param T the type of the value needed
	 * @param array the storage
	 * @throw whatever is thrown
	 */
	template <typename T>
	Option<std::vector<T>> getArray()
		{
			try {
				std::vector<T> array;
				array.reserve(m_count);
				for (uint32_t i = 0; i < m_count; i++) {
					array.push_back(IfdTypeTrait<T>::get(*this, i));
				}
				return Option<decltype(array)>(array);
			}
			catch(const std::exception & e)
			{
				LOGERR("Exception: %s\n", e.what());
			}
			return Option<std::vector<T>>();
		}
	uint32_t getIntegerArrayItem(int idx);

private:
	uint16_t m_id;
	uint16_t m_type;
	uint32_t m_count;
	uint32_t m_data; /**< raw data without endian conversion */
	bool m_loaded;
	uint8_t *m_dataptr;
	IfdFileContainer & m_container;
	template <typename T> friend struct IfdTypeTrait;

	/** private copy constructor to make sure it is not called */
	IfdEntry(const IfdEntry& f);
	/** private = operator to make sure it is never called */
	IfdEntry & operator=(const IfdEntry&);

};



/** get the value of type T
 * @param T the type of the value needed
 * @param idx the index, by default 0
 * @param ignore_type if true, don't check type. *DANGEROUS* Default is false.
 * @return the value
 * @throw BadTypeException in case of wrong typing.
 * @throw OutOfRangeException in case of subscript out of range
 */
template <typename T>
T IfdTypeTrait<T>::get(IfdEntry & e, uint32_t idx, bool ignore_type)
	noexcept(false)
{
	/* format undefined means that we don't check the type */
	if(!ignore_type && (e.m_type != IFD::EXIF_FORMAT_UNDEFINED)) {
		if (e.m_type != IfdTypeTrait<T>::type) {
			throw BadTypeException();
		}
	}
	if (idx + 1 > e.m_count) {
		throw OutOfRangeException();
	}
	if (!e.m_loaded) {
		e.m_loaded = e.loadData(IfdTypeTrait<T>::size);
		if (!e.m_loaded) {
			throw TooBigException();
		}
	}
	uint8_t *data;
	if (e.m_dataptr == NULL) {
		data = (uint8_t*)&e.m_data;
	}
	else {
		data = e.m_dataptr;
	}
	data += (IfdTypeTrait<T>::size * idx);
	T val;
	if (e.endian() == RawContainer::ENDIAN_LITTLE) {
		val = IfdTypeTrait<T>::EL(data, e.m_count - idx);
	}
	else {
		val = IfdTypeTrait<T>::BE(data, e.m_count - idx);
	}
	return val;
}


}
}


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
#endif


