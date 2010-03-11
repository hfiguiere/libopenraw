/*
 * libopenraw - dngfile.cpp
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


#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include <boost/scoped_ptr.hpp>

#include "trace.h"
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
		const IFDFile::camera_ids_t DNGFile::s_def[] = {
			{ "PENTAX K10D        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
														 OR_TYPEID_PENTAX_K10D_DNG) },
			{ "R9 - Digital Back DMR",   OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
													   OR_TYPEID_LEICA_DMR) },
			{ "M8 Digital Camera",       OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
													   OR_TYPEID_LEICA_M8) },
			{ "LEICA X1               ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
													   OR_TYPEID_LEICA_X1) },			
			{ "GR DIGITAL 2   ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
													 OR_TYPEID_RICOH_GR2) },
			{ "GXR            ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
													 OR_TYPEID_RICOH_GXR) },
			{ "SAMSUNG GX10       ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SAMSUNG,
														 OR_TYPEID_SAMSUNG_GX10) },
			{ "Pro 815    ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SAMSUNG, 
												 OR_TYPEID_SAMSUNG_PRO815) },
			{ 0, OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_ADOBE, 
									 OR_TYPEID_ADOBE_DNG_GENERIC) }
		};

		RawFile *DNGFile::factory(IO::Stream *s)
		{
			return new DNGFile(s);
		}


		DNGFile::DNGFile(IO::Stream *s)
			: TiffEpFile(s, OR_RAWFILE_TYPE_DNG)
		{
			_setIdMap(s_def);
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
