/*
 * libopenraw - cr2file.cpp
 *
 * Copyright (C) 2006-2010 Hubert Figuiere
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

#include <boost/scoped_ptr.hpp>
#include <boost/any.hpp>
#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
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

const IfdFile::camera_ids_t Cr2File::s_def[] = {
	{ "Canon EOS-1D Mark II", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
												  OR_TYPEID_CANON_1DMKII) },
	{ "Canon EOS-1D Mark III", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
												   OR_TYPEID_CANON_1DMKIII) },
	{ "Canon EOS-1D Mark IV", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
													OR_TYPEID_CANON_1DMKIV) },
	{ "Canon EOS-1Ds Mark II", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
												   OR_TYPEID_CANON_1DSMKII) },
	{ "Canon EOS-1Ds Mark III", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
													OR_TYPEID_CANON_1DSMKIII) },
	{ "Canon EOS 20D" , OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
											OR_TYPEID_CANON_20D) },
	{ "Canon EOS 20Da", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
											OR_TYPEID_CANON_20DA) },
	{ "Canon EOS 30D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										   OR_TYPEID_CANON_30D) },
	{ "Canon EOS 350D DIGITAL", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
											OR_TYPEID_CANON_350D) },			
	{ "Canon EOS DIGITAL REBEL XT", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
											OR_TYPEID_CANON_350D) },			
	{ "Canon EOS 40D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										   OR_TYPEID_CANON_40D) },
	{ "Canon EOS 400D DIGITAL", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										  OR_TYPEID_CANON_400D) },
	{ "Canon EOS 450D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										  OR_TYPEID_CANON_450D) },
	{ "Canon EOS 50D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										   OR_TYPEID_CANON_50D) },
	{ "Canon EOS 500D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										  OR_TYPEID_CANON_500D) },
	{ "Canon EOS 550D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
											OR_TYPEID_CANON_550D) },
	{ "Canon EOS 1000D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
											OR_TYPEID_CANON_1000D) },
	{ "Canon EOS 5D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										  OR_TYPEID_CANON_5D) },
	{ "Canon EOS 5D Mark II", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										  OR_TYPEID_CANON_5DMKII) },
	{ "Canon EOS 7D", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,
										  OR_TYPEID_CANON_7D) },
	{ "Canon PowerShot G9", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, 
												OR_TYPEID_CANON_G9) },
	{ "Canon PowerShot G10", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, 
												OR_TYPEID_CANON_G11) },
	{ "Canon PowerShot G11", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, 
												OR_TYPEID_CANON_G11) },
	{ 0, 0 }
};

RawFile *Cr2File::factory(IO::Stream * s)
{
	return new Cr2File(s);
}

Cr2File::Cr2File(IO::Stream * s)
	: IfdFile(s, OR_RAWFILE_TYPE_CR2)
{
	_setIdMap(s_def);
}

Cr2File::~Cr2File()
{
}


IfdDir::Ref  Cr2File::_locateCfaIfd()
{
	return m_container->setDirectory(3);
}


IfdDir::Ref  Cr2File::_locateMainIfd()
{
	return m_container->setDirectory(0);
}

::or_error Cr2File::_getRawData(RawData & data, uint32_t options)
{
	::or_error ret = OR_ERROR_NONE;
	const IfdDir::Ref & _cfaIfd = cfaIfd();
	if(!_cfaIfd) {
		Trace(DEBUG1) << "cfa IFD not found\n";
		return OR_ERROR_NOT_FOUND;
	}

	Trace(DEBUG1) << "_getRawData()\n";
	uint32_t offset = 0;
	uint32_t byte_length = 0;
	bool got_it;
	got_it = _cfaIfd->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
	if(!got_it) {
		Trace(DEBUG1) << "offset not found\n";
		return OR_ERROR_NOT_FOUND;
	}
	got_it = _cfaIfd->getValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_length);
	if(!got_it) {
		Trace(DEBUG1) << "byte len not found\n";
		return OR_ERROR_NOT_FOUND;
	}
	// get the "slicing", tag 0xc640 (3 SHORT)
	std::vector<uint16_t> slices;
	IfdEntry::Ref e = _cfaIfd->getEntry(IFD::CR2_TAG_SLICE);
	if (e) {
		e->getArray(slices);
		Trace(DEBUG1) << "Found slice entry " << slices << "\n";
	}

	const IfdDir::Ref & _exifIfd = exifIfd();
	if (_exifIfd) {
		uint16_t x, y;
		x = 0;
		y = 0;
		got_it = _exifIfd->getValue(IFD::EXIF_TAG_PIXEL_X_DIMENSION, x);
		if(!got_it) {
			Trace(DEBUG1) << "X not found\n";
			return OR_ERROR_NOT_FOUND;
		}
		got_it = _exifIfd->getValue(IFD::EXIF_TAG_PIXEL_Y_DIMENSION, y);
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
		// they are not all RGGB.
		// but I don't seem to see where this is encoded.
		// 
		data.setCfaPattern(OR_CFA_PATTERN_RGGB);
		data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);
		data.setDimensions(x, y);

		Trace(DEBUG1) << "In size is " << data.width() 
					  << "x" << data.height() << "\n";
		// decompress if we need
		if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
			boost::scoped_ptr<IO::Stream> s(new IO::MemStream(data.data(),
															  data.size()));
			s->open(); // TODO check success
			boost::scoped_ptr<JFIFContainer> jfif(new JFIFContainer(s.get(), 0));
			LJpegDecompressor decomp(s.get(), jfif.get());
			// in fact on Canon CR2 files slices either do not exists
			// or is 3.
			if(slices.size() > 1) {
				decomp.setSlices(slices); 
			}
			RawData *dData = decomp.decompress();
			if (dData != NULL) {
				Trace(DEBUG1) << "Out size is " << dData->width() 
											<< "x" << dData->height() << "\n";
				// must re-set the cfaPattern
				dData->setCfaPattern(data.cfaPattern());
				data.swap(*dData);
				delete dData;
			}
		}
		
		// get the sensor info
		std::vector<uint16_t> sensorInfo;
		const IfdDir::Ref & _makerNoteIfd = makerNoteIfd();
		e = _makerNoteIfd->getEntry(IFD::MNOTE_CANON_SENSORINFO);
		if(e) {
			e->getArray(sensorInfo);
			uint32_t w = sensorInfo[7] - sensorInfo[5];
			uint32_t h = sensorInfo[8] - sensorInfo[6];
			data.setRoi(sensorInfo[5], sensorInfo[6], w, h);
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
