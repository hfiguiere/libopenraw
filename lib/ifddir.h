/*
 * libopenraw - ifddir.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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


#ifndef _OPENRAW_INTERNALS_IFDDIR_H
#define _OPENRAW_INTERNALS_IFDDIR_H

#include <map>

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include "ifdentry.h"
#include "debug.h"

namespace OpenRaw {
	namespace Internals {
		
		class IFDFileContainer;

		class IFDDir
		{
		public:
			typedef boost::shared_ptr<IFDDir> Ref;
			typedef std::vector<Ref> RefVec;
			struct isPrimary
			{
				bool operator()(const Ref &dir);
			};
			struct isThumbnail
			{
				bool operator()(const Ref &dir);
			};

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
			IFDEntry::Ref getEntry(uint16_t id) const ;
			
			/** Get a T value from an entry
			 * @param id the IFD field id
			 * @retval v the long value
			 * @return true if success
			 */
			template <typename T>
			bool getValue(uint16_t id, T &v) const
				{
					bool success = false;
					IFDEntry::Ref e = getEntry(id);
					if (e != NULL) {
						try {
							v = IFDTypeTrait<T>::get(*e);
							success = true;
						}
						catch(const std::exception & ex) {
							Debug::Trace(ERROR) << "Exception raised " << ex.what() 
													 << " fetch value for " << id << "\n";
						}
					}
					return success;
				}

			/** Get an loosely typed integer value from an entry.
			 * This method is  preferred over getLongValue() 
			 * or getShortValue() unless you really want the strong 
			 * typing that IFD structure provide
			 * @param id the IFD field id
			 * @retval v the long value
			 * @return true if success
			 */
			bool getIntegerValue(uint16_t id, uint32_t &v);

			/** get the offset of the next IFD 
			 * in absolute
			 */
			off_t nextIFD();

			/** get the SubIFD at index idx.
			 * @return Ref to the new IFDDir if found
			 */
			Ref getSubIFD(uint32_t idx = 0) const;
			/** get all SubIFDs 
			 * @retval ifds the list of IFDs Ref	
			 * @return true if found / success
			 */
			bool getSubIFDs(std::vector<IFDDir::Ref> & ifds);

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

