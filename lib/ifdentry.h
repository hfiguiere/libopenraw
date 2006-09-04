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

namespace OpenRaw {
	namespace Internals {

		class IFDFileContainer;

		class IFDEntry
		{
		public:
			/** Ref (ie shared pointer) */
			typedef boost::shared_ptr<IFDEntry> Ref;

			IFDEntry(Int16 _id, Int16 _type, Int32 _count, Int32 _offset,
							 IFDFileContainer &_container);
			virtual ~IFDEntry();


			Int32 getLong();
		private:
			Int16 m_id;
			Int16 m_type;
			Int32 m_count;
			Int32 m_offset;
			IFDFileContainer & m_container;
		};


	}
}


#endif


