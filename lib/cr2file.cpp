/*
 * libopenraw - cr2file.cpp
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

#include <boost/scoped_ptr.hpp>
#include <boost/any.hpp>
#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "debug.h"
#include "io/file.h"
#include "io/memstream.h"
#include "ifdfilecontainer.h"
#include "ifd.h"
#include "cr2file.h"
#include "jfifcontainer.h"
#include "ljpegdecompressor.h"
#include "rawfilefactory.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		RawFile *CR2File::factory(const char* _filename)
		{
			return new CR2File(_filename);
		}

		CR2File::CR2File(const char* _filename)
			: IFDFile(_filename, OR_RAWFILE_TYPE_CR2)
		{

		}

		CR2File::~CR2File()
		{
		}


		IFDDir::Ref  CR2File::_locateCfaIfd()
		{
			return m_container->setDirectory(3);
		}

		IFDDir::Ref  CR2File::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}

		::or_error CR2File::_getRawData(RawData & data, uint32_t options)
		{
			::or_error ret = OR_ERROR_NONE;
			if(!m_cfaIfd) {
				m_cfaIfd = _locateCfaIfd();
			}
			if(!m_cfaIfd) {
				Trace(DEBUG1) << "cfa IFD not found\n";
				return OR_ERROR_NOT_FOUND;
			}

			Trace(DEBUG1) << "_getRawData()\n";
			uint32_t offset = 0;
			uint32_t byte_length = 0;
			bool got_it;
			got_it = m_cfaIfd->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
			if(!got_it) {
				Trace(DEBUG1) << "offset not found\n";
				return OR_ERROR_NOT_FOUND;
			}
			got_it = m_cfaIfd->getValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_length);
			if(!got_it) {
				Trace(DEBUG1) << "byte len not found\n";
				return OR_ERROR_NOT_FOUND;
			}
			// get the "slicing", tag 0xc640 (3 SHORT)
			std::vector<uint16_t> slices;
			IFDEntry::Ref e = m_cfaIfd->getEntry(IFD::EXIF_TAG_CR2_SLICE);
			if (e) {
				e->getArray(slices);
				Trace(DEBUG1) << "Found slice entry " << slices << "\n";
			}

			if(!m_exifIfd) {
				m_exifIfd = _locateExifIfd();
			}
			if (m_exifIfd) {
				uint16_t x, y;
				x = 0;
				y = 0;
				got_it = m_exifIfd->getValue(IFD::EXIF_TAG_PIXEL_X_DIMENSION, x);
				if(!got_it) {
					Trace(DEBUG1) << "X not found\n";
					return OR_ERROR_NOT_FOUND;
				}
				got_it = m_exifIfd->getValue(IFD::EXIF_TAG_PIXEL_Y_DIMENSION, y);
				if(!got_it) {
					Trace(DEBUG1) << "Y not found\n";
					return OR_ERROR_NOT_FOUND;
				}
				
				void *p = data.allocData(byte_length);
				size_t real_size = m_container->fetchData(p, offset, 
														  byte_length);
				if (real_size < byte_length) {
					Trace(WARNING) << "Size mismatch for data: ignoring.\n";
				}
				data.setCfaPattern(OR_CFA_PATTERN_RGGB);
				data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);
				data.setDimensions(x, y);
				Trace(DEBUG1) << "In size is " << data.x() 
							  << "x" << data.y() << "\n";
				// decompress if we need
				if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
					boost::scoped_ptr<IO::Stream> s(new IO::MemStream(data.data(),
																	  data.size()));
					s->open(); // TODO check success
					boost::scoped_ptr<JFIFContainer> jfif(new JFIFContainer(s.get(), 0));
					LJpegDecompressor decomp(s.get(), jfif.get());
					if(slices.size() > 1) {
						decomp.setSlices(slices, 1); 
					}
					RawData *dData = decomp.decompress();
					if (dData != NULL) {
						Trace(DEBUG1) << "Out size is " << dData->x() 
													<< "x" << dData->y() << "\n";
						// must re-set the cfaPattern
						dData->setCfaPattern(data.cfaPattern());
						data.swap(*dData);
						delete dData;
					}
				}
			}
			else {
				Trace(ERROR) << "unable to find ExifIFD\n";
				ret = OR_ERROR_NOT_FOUND;
			}
			return ret;
		}


	}
}
