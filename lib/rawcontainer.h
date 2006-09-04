/*
 * libopenraw - rawcontainer.h
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




#ifndef _RAWCONTAINER_H_
#define _RAWCONTAINER_H_

#include <sys/types.h>
#include <libopenraw/io.h>

namespace OpenRaw {
	namespace Internals {

		
		class IOFile;
		
/**
   Generic interface for the RAW file container
 */
		class RawContainer
		{
		public:
			/** 
					@param file the file handle
					@param offset the offset since starting the 
					begining of the file for the container
			*/
			RawContainer(IOFile *file, off_t offset);
			/** destructor */
			virtual ~RawContainer();
			
			IOFile *file()
				{
					return m_file;
				}
		protected:

			RawContainer(const RawContainer&);
			RawContainer & operator=(const RawContainer &);

			/** the file handle */
			IOFile *m_file;
			/** the offset from the begining of the file */
			off_t m_offset;
		};
		
	}
}


#endif
