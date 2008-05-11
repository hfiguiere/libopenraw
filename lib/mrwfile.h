/*
 * libopenraw - mrwfile.h
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
 * Copyright (C) 2008 Bradley Broom
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




#ifndef __MRWFILE_H_
#define __MRWFILE_H_

#include "ifdfile.h"

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {

		class MRWFile
			: public IFDFile
		{
		public:
			static RawFile *factory(IO::Stream* _filename);
			MRWFile(IO::Stream* _filename);
			virtual ~MRWFile();

		protected:
			virtual IFDDir::Ref  _locateCfaIfd();
			virtual IFDDir::Ref  _locateMainIfd();

			virtual void _identifyId();
			
			virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list);
			virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail);
			virtual ::or_error _getRawData(RawData & data, uint32_t options);

		private:

			MRWFile(const MRWFile&);
			MRWFile & operator=(const MRWFile&);

			static const struct IFDFile::camera_ids_t s_def[];
		};
	}

}

#endif
