/*
 * libopenraw - ifddir.h
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


#ifndef _OPENRAW_INTERNALS_IFDDIR_H
#define _OPENRAW_INTERNALS_IFDDIR_H

#include <map>

#include <boost/shared_ptr.hpp>
#include "ifdentry.h"

namespace OpenRaw {
	namespace Internals {
		
		class IFDFileContainer;

		class IFDDir
		{
		public:
			typedef boost::shared_ptr<IFDDir> Ref;

			IFDDir(off_t _offset, IFDFileContainer & _container);
			virtual ~IFDDir();
			
      /** return the offset */
			off_t offset() const
				{
					return m_offset;
				}

			/** load the directory to memory */
			bool load();
			/** return the number of entries*/
			int numTags()
				{
					return m_entries.size();
				}
			IFDEntry::Ref getEntry(int id);

			/** Get an loosely typed integer value from an entry.
			 * This method is  preferred over getLongValue() 
			 * or getShortValue() unless you really want the strong 
			 * typing that IFD structure provide
			 * @param id the IFD field id
			 * @retval v the long value
			 * @return true if success
			 */
			bool getIntegerValue(int id, uint32_t &v);
			/** Get a long value from an entry
			 * @param id the IFD field id
			 * @retval v the long value
			 * @return true if success
			 */
			bool getLongValue(int id, uint32_t &v);
			/** Get a short value from an entry
			 * @param id the IFD field id
			 * @retval v the long value
			 * @return true if success
			 */
			bool getShortValue(int id, uint16_t &v);
			/** get the offset of the next IFD 
			 * in absolute
			 */
			off_t nextIFD();

			/** get the SubIFD.
			 * @return Ref to the new IFDDir if found
			 */
			Ref getSubIFD();

			/** get the Exif IFD.
			 * @return Ref to the new IFDDir if found
			 */
			Ref getExifIFD();
		private:
			off_t m_offset;
			IFDFileContainer & m_container;
			std::map<uint16_t, IFDEntry::Ref> m_entries;
		};


	}
}


#endif

