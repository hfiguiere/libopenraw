/*
 * libopenraw - neffile.h
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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

#ifndef __NEFFILE_H_
#define __NEFFILE_H_

#include "tiffepfile.h"
#include "huffman.h"
#include <vector>

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {
		class IOFile;
		class IFDFileContainer;

		class NEFFile
			: public TiffEpFile
		{
		public:
			static RawFile *factory(IO::Stream* _f);
			NEFFile(IO::Stream * _f);
			virtual ~NEFFile();

			/** hack because some (lot?) D100 do set as compressed even though 
			 *  it is not
			 */
			static bool isCompressed(RawContainer & container, uint32_t offset);

			class NEFCompressionInfo {
			public:
			  uint16_t vpred[2][2];
			  std::vector<uint16_t> curve;
			  const HuffmanNode* huffman;
			};
		private:

			NEFFile(const NEFFile&);
			NEFFile & operator=(const NEFFile &);

			virtual ::or_error _getRawData(RawData & data, uint32_t options);
			static const IFDFile::camera_ids_t s_def[];
			int _getCompressionCurve(RawData&, NEFCompressionInfo&);
			::or_error _decompressNikonQuantized(RawData&);
			::or_error _decompressIfNeeded(RawData&, uint32_t);
		};
	}

}

#endif
