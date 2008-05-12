/*
 * libopenraw - dngfile.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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




#ifndef __DNGFILE_H_
#define __DNGFILE_H_

#include "tiffepfile.h"

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {
		class IOFile;
		class IFDFileContainer;

		class DNGFile
			: public TiffEpFile
		{
		public:
			static RawFile *factory(IO::Stream *);

			DNGFile(IO::Stream *);
			virtual ~DNGFile();

		protected:
			virtual ::or_error _getRawData(RawData & data, uint32_t options);

		private:

			DNGFile(const DNGFile&);
			DNGFile & operator=(const DNGFile&);

			static const IFDFile::camera_ids_t s_def[];
		};
	}

}

#endif
