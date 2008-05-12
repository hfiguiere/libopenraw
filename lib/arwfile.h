/*
 * libopenraw - arwfile.h
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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




#ifndef __ARWFILE_H_
#define __ARWFILE_H_

#include "ifdfile.h"

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {
		class IOFile;
		class IFDFileContainer;

		class ARWFile
			: public IFDFile
		{
		public:
			static RawFile *factory(IO::Stream* s);
			ARWFile(IO::Stream * s);
			virtual ~ARWFile();

		protected:
			virtual IFDDir::Ref  _locateCfaIfd();
			virtual IFDDir::Ref  _locateMainIfd();

			virtual ::or_error _getRawData(RawData & data, uint32_t options);

		private:

			ARWFile(const ARWFile&);
			ARWFile & operator=(const ARWFile&);

			static const IFDFile::camera_ids_t s_def[];
		};
	}

}

#endif
