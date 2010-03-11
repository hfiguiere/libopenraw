/* -*- tab-width:4; c-basic-offset:4 -*- */

/*
 * libopenraw - neffile.cpp
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


#include <iostream>
#include <vector>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "io/file.h"
#include "huffman.h"
#include "nefdiffiterator.h"
#include "nefcfaiterator.h"
#include "neffile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {
		const IFDFile::camera_ids_t NEFFile::s_def[] = {
			{ "NIKON D1 ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D1) },
			{ "NIKON D100 ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D100) },
			{ "NIKON D1X", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D1X) },
			{ "NIKON D200", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
												OR_TYPEID_NIKON_D200) },
			{ "NIKON D2H", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D2H ) },
			{ "NIKON D2X", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D2X ) },
			{ "NIKON D3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											  OR_TYPEID_NIKON_D3) },
			{ "NIKON D300", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
												OR_TYPEID_NIKON_D300) },
			{ "NIKON D3000", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
												OR_TYPEID_NIKON_D3000) },
			{ "NIKON D40", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D40) },
			{ "NIKON D40X", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D40X) },
			{ "NIKON D50", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D50) },
			{ "NIKON D70", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D70) },
			{ "NIKON D70s", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D70S) },
			{ "NIKON D80", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D80) },
			{ 0, 0 }
		};

		RawFile *NEFFile::factory(IO::Stream* _filename)
		{
			return new NEFFile(_filename);
		}

		NEFFile::NEFFile(IO::Stream* _filename)
			: TiffEpFile(_filename, OR_RAWFILE_TYPE_NEF)
		{
			_setIdMap(s_def);
		}


		NEFFile::~NEFFile()
		{
		}

		bool NEFFile::isCompressed(RawContainer & container, uint32_t offset)
		{
			int i;
			uint8_t buf[256];
			size_t real_size = container.fetchData(buf, offset, 
												   256);
			if(real_size != 256) {
				return true;
			}
			for(i = 15; i < 256; i+= 16) {
				if(buf[i]) {
					Trace(DEBUG1) << "isCompressed: true\n";
					return true;
				}
			}
			Trace(DEBUG1) << "isCompressed: false\n";
			return false;
		}

		::or_error NEFFile::_decompressNikonQuantized(RawData & data)
		{
			NEFCompressionInfo c;
			if (!_getCompressionCurve(data, c)) {
				return OR_ERROR_NOT_FOUND;
			}
			const uint32_t rows = data.y();
			const uint32_t raw_columns = data.x();

			//FIXME: not always true
			const uint32_t columns = raw_columns - 1;

			NefDiffIterator
				diffs(c.huffman, data.data());
			NefCfaIterator iter(diffs, rows, raw_columns, c.vpred);

			RawData newData;
			uint16_t *p = (uint16_t *) newData.allocData(rows * columns * 2);
			newData.setDimensions(columns, rows);
			newData.setDataType(OR_DATA_TYPE_CFA);
            uint16_t bpc = data.bpc();
			newData.setBpc(bpc);
            newData.setMax((1 << bpc) - 1);
			newData.setCfaPattern(data.cfaPattern());
	   
			for (unsigned int i = 0; i < rows; i++) {
				for (unsigned int j = 0; j < raw_columns; j++) {
					uint16_t t = iter.get();
					if (j < columns) {
						unsigned shift = 16 - data.bpc();
						p[i * columns + j] =  c.curve[t & 0x3fff] << shift;
					}
				}
			}

			data.swap(newData);
			return OR_ERROR_NONE;
		}

		::or_error NEFFile::_decompressIfNeeded(RawData & data,
												uint32_t options)
		{
			uint32_t compression = data.compression();
			if((options & OR_OPTIONS_DONT_DECOMPRESS) ||
			   compression == IFD::COMPRESS_NONE) {
				return OR_ERROR_NONE;
			} else if(compression == IFD::COMPRESS_NIKON_QUANTIZED) {
				return _decompressNikonQuantized(data);
			} else {
				return OR_ERROR_INVALID_FORMAT;
			}
		}

		int NEFFile::_getCompressionCurve(RawData & data,  NEFFile::NEFCompressionInfo& c)
		{
			if(!m_exifIfd) {
				m_exifIfd = _locateExifIfd();
			}
			if(!m_exifIfd) {
				return 0;
			}

			IFDEntry::Ref maker_ent =
				m_exifIfd->getEntry(IFD::EXIF_TAG_MAKER_NOTE);
			if(!maker_ent) {
				return 0;
			}

			uint32_t off = maker_ent->offset();
			uint32_t base = off + 10;

			IFDDir::Ref ref(new IFDDir(base + 8, *m_container));
			ref->load();
			IFDEntry::Ref curveEntry = ref->getEntry(0x0096);
			if(!curveEntry) {
				return 0;
			}

			size_t pos = base + curveEntry->offset();

			IO::Stream *file = m_container->file();
			file->seek(pos, SEEK_SET);

			int16_t aux;

			uint16_t header;
			bool read = m_container->readInt16(file, aux);
			header = aux;
			if(!read) {
				return 0;
			}

			if (header == 0x4410) {
				c.huffman = NefDiffIterator::Lossy12Bit;
				data.setBpc(12);
			} else if (header == 0x4630) {
				c.huffman = NefDiffIterator::LossLess14Bit;
				data.setBpc(14);
			} else {
				return 0;
			}

			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					read = m_container->readInt16(file, aux);
					if(!read) {
						return 0;
					}
					c.vpred[i][j] = aux;
				}
			}

			if (header == 0x4410) {
				size_t nelems;
				read = m_container->readInt16(file, aux);
				nelems = aux;

				for (size_t i = 0; i < nelems; ++i) {
					read = m_container->readInt16(file, aux);
					if (!read)
						return 0;
					c.curve.push_back(aux);
				}
			} else if (header == 0x4630) {
				for (size_t i = 0; i <= 0x3fff; ++i) {
					c.curve.push_back(i);
				}
			}

			return 1;
		}

		::or_error NEFFile::_getRawData(RawData & data, uint32_t options)
		{
			::or_error ret = OR_ERROR_NONE;
			m_cfaIfd = _locateCfaIfd();
			Trace(DEBUG1) << "_getRawData()\n";

			if(m_cfaIfd) {
				ret = _getRawDataFromDir(data, m_cfaIfd);
				if (ret != OR_ERROR_NONE) {
					return ret;
				}
				ret = _decompressIfNeeded(data, options);
			}
			else {
				ret = OR_ERROR_NOT_FOUND;
			}
			return ret;
		}

	}
}

