/*
 * libopenraw - orffile.h
 *
 * Copyright (C) 2006-2008, 2010, 2012 Hubert Figuiere
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




#ifndef __ORFFILE_H_
#define __ORFFILE_H_

#include "ifdfile.h"

namespace OpenRaw {

	class Thumbnail;
	class RawData;

	namespace Internals {

		class OrfFile
			: public IfdFile
		{
		public:
			static RawFile *factory(IO::Stream *);
			OrfFile(IO::Stream *);
			virtual ~OrfFile();
			
			enum {
				ORF_COMPRESSION = 0x10000
			};

		protected:
			virtual IfdDir::Ref  _locateCfaIfd();
			virtual IfdDir::Ref  _locateMainIfd();

			virtual ::or_error _getRawData(RawData & data, uint32_t options);
			virtual uint32_t _translateCompressionType(IFD::TiffCompress tiffCompression);
			virtual ::or_error _getColourMatrix(uint32_t index, double* matrix, uint32_t & size);
		private:
			static RawFile::TypeId _typeIdFromModel(const std::string & model);

			OrfFile(const OrfFile&);
			OrfFile & operator=(const OrfFile &);

			static const IfdFile::camera_ids_t s_def[];
		};
	}

}

#endif
