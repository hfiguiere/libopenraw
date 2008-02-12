/*
 * libopenraw - dngfile.cpp
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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


#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include <boost/scoped_ptr.hpp>

#include "debug.h"
#include "io/file.h"
#include "io/memstream.h"
#include "ifdfilecontainer.h"
#include "jfifcontainer.h"
#include "ljpegdecompressor.h"
#include "ifd.h"
#include "dngfile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *DNGFile::factory(const char* _filename)
		{
			return new DNGFile(_filename);
		}


		DNGFile::DNGFile(const char* _filename)
			: TiffEpFile(_filename, OR_RAWFILE_TYPE_DNG)
		{

		}

		DNGFile::~DNGFile()
		{
		}

		::or_error DNGFile::_getRawData(RawData & data, uint32_t options)
		{
			::or_error ret = OR_ERROR_NONE;
			if(!m_cfaIfd) {
				m_cfaIfd = _locateCfaIfd();
			}

			Trace(DEBUG1) << "_getRawData()\n";

			if (m_cfaIfd) {
				ret = _getRawDataFromDir(data, m_cfaIfd);
				
				if(ret == OR_ERROR_NONE) {
					uint16_t compression = 0;
					if (m_cfaIfd->getValue(IFD::EXIF_TAG_COMPRESSION, compression) &&
						compression == 7) {
						// if the option is not set, decompress
						if ((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
							boost::scoped_ptr<IO::Stream> s(new IO::MemStream(data.data(),
																			  data.size()));
							s->open(); // TODO check success
							boost::scoped_ptr<JFIFContainer> jfif(new JFIFContainer(s.get(), 0));
							LJpegDecompressor decomp(s.get(), jfif.get());
							RawData *dData = decomp.decompress();
							if (dData != NULL) {
								dData->setCfaPattern(data.cfaPattern());
								data.swap(*dData);
								delete dData;
							}
						}
					}
					else {
						data.setDataType(OR_DATA_TYPE_CFA);
					}
				}
				else {
					Trace(ERROR) << "couldn't find raw data\n";
				}
			}
			else {
				ret = OR_ERROR_NOT_FOUND;
			}
			return ret;
		}

	}
}
