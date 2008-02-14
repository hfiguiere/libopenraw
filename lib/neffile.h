/*
 * libopenraw - neffile.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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




#ifndef __NEFFILE_H_
#define __NEFFILE_H_

#include "tiffepfile.h"

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {
		class IOFile;
		class IFDFileContainer;

		class NEFFile
			: public TiffEpFile
		{
		public:
			static RawFile *factory(const char* _filename);
			NEFFile(const char* _filename);
			virtual ~NEFFile();

			/** hack because some (lot?) D100 do set as compressed even though 
			 *  it is not
			 */
			static bool isCompressed(RawContainer & container, uint32_t offset);
		private:

			NEFFile(const NEFFile&);
			NEFFile & operator=(const NEFFile &);

			virtual ::or_error _getRawData(RawData & data, uint32_t options);
		};
	}

}

#endif
