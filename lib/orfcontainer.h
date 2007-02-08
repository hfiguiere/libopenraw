/*
 * libopenraw - orfcontainer.h
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#ifndef _ORF_CONTAINER_H__
#define _ORF_CONTAINER_H__

#include "ifdfilecontainer.h"

namespace OpenRaw {
	namespace Internals {

		class IOFile;

		class ORFContainer
			: public IFDFileContainer
		{
		public:
			ORFContainer(IO::Stream *file, off_t offset);
			/** destructor */
			virtual ~ORFContainer();

			/**
				 Check the ORF magic header.
			 */
			virtual IFDFileContainer::EndianType 
			isMagicHeader(const char *p, int len);

		private:
			/* avoid these being called. */
			ORFContainer(const ORFContainer &);
			ORFContainer & operator=(const ORFContainer &);
		};

	}
}


#endif
