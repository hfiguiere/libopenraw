/*
 * libopenraw - ifdentry.h
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


#ifndef _OPENRAW_INTERNALS_IFDENTRY_H
#define _OPENRAW_INTERNALS_IFDENTRY_H

#include <boost/shared_ptr.hpp>
#include <libopenraw/types.h>

#include "exception.h"

namespace OpenRaw {
	namespace Internals {

		class IFDFileContainer;

		class IFDEntry
		{
		public:
			/** Ref (ie shared pointer) */
			typedef boost::shared_ptr<IFDEntry> Ref;

			IFDEntry(int16_t _id, int16_t _type, int32_t _count, uint32_t _data,
							 IFDFileContainer &_container);
			virtual ~IFDEntry();

			int16_t type() const
				{
					return m_type;
				}

			uint32_t getLong() throw (BadTypeException, TooBigException);
			uint16_t getShort() throw (BadTypeException, TooBigException);
		private:
			int16_t m_id;
			int16_t m_type;
			int32_t m_count;
			uint32_t m_data; /**< raw data without endian conversion */
			IFDFileContainer & m_container;
		};


	}
}


#endif


