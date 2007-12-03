/*
 * libopenraw - metavalue.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include "exception.h"
#include "metavalue.h"

namespace OpenRaw {

	MetaValue::MetaValue(const MetaValue & r)
		: m_value(r.m_value)
	{
	}

	MetaValue::MetaValue(const boost::any &v)
		: m_value(v)
	{
	}


	int32_t MetaValue::getInteger() const
		throw(Internals::BadTypeException)
	{
		int32_t v = 0;
		try {
			v = boost::any_cast<int32_t>(m_value);
		}
		catch(const boost::bad_any_cast &) {
			throw Internals::BadTypeException();
		}
		return v;
	}

}
