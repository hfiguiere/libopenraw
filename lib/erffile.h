/*
 * libopenraw - erffile.h
 *
 * Copyright (C) 2007-2008 Hubert Figuiere
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




#ifndef __ERFFILE_H_
#define __ERFFILE_H_

#include "tiffepfile.h"

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {
		class IOFile;
		class IFDFileContainer;

		class ERFFile
			: public TiffEpFile
		{
		public:
			static RawFile *factory(IO::Stream *);
			ERFFile(IO::Stream *);
			virtual ~ERFFile();

		protected:
			virtual ::or_error _getRawData(RawData & data, uint32_t options);

		private:

			ERFFile(const ERFFile&);
			ERFFile & operator=(const ERFFile &);

			static const IFDFile::camera_ids_t s_def[];
		};
	}

}

#endif
