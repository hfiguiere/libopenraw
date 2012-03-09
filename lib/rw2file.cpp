/*
 * libopenraw - rw2file.cpp
 *
 * Copyright (C) 2011-2012 Hubert Figuiere
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
#include "io/streamclone.h"
#include "ifd.h"
#include "rw2file.h"
#include "rw2container.h"
#include "jfifcontainer.h"
#include "rawfilefactory.h"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

const IfdFile::camera_ids_t Rw2File::s_def[] = {
	{ "DMC-GF1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_GF1) },
	{ "DMC-GF2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_GF2) },
	{ "DMC-GF3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_GF3) },
	{ "DMC-GX1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_GX1) },
	{ "DMC-FZ8", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_FZ8) },
	{ "DMC-FZ18", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_FZ18) },
	{ "DMC-FZ28", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_FZ28) },
	{ "DMC-FZ30", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_FZ30) },
	{ "DMC-FZ50", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_FZ50) },
	{ "DMC-FZ100", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_FZ100) },
	{ "DMC-G1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_G1) },
	{ "DMC-G2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_G2) },
	{ "DMC-G3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_G3) },
	{ "DMC-G10", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_G10) },
	{ "DMC-GH1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_GH1) },
	{ "DMC-GH2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_GH2) },
	{ "DMC-LX2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_LX2) },
	{ "DMC-LX3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_LX3) },
	{ "DMC-LX5", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_LX5) },
	{ "DMC-L1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
						  OR_TYPEID_PANASONIC_L1) },
	{ "DMC-L10", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,
									OR_TYPEID_PANASONIC_L10) },
	{ "DIGILUX 2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
									 OR_TYPEID_LEICA_DIGILUX2) },
	{ "D-LUX 3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
									 OR_TYPEID_LEICA_DLUX_3) },
	{ "V-LUX 1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
									 OR_TYPEID_LEICA_VLUX_1) },
	
	{ 0, 0 }
};

RawFile *Rw2File::factory(IO::Stream * s)
{
	return new Rw2File(s);
}

Rw2File::Rw2File(IO::Stream * s)
	: IfdFile(s, OR_RAWFILE_TYPE_RW2, false)
{
	_setIdMap(s_def);
	m_container = new Rw2Container(m_io, 0);
}

Rw2File::~Rw2File()
{
}


IfdDir::Ref  Rw2File::_locateCfaIfd()
{
	return mainIfd();
}


IfdDir::Ref  Rw2File::_locateMainIfd()
{
	return m_container->setDirectory(0);
}

::or_error Rw2File::_locateThumbnail(const IfdDir::Ref & dir,
                                     std::vector<uint32_t> &list)
{
    uint32_t offset = 0;
    uint32_t size = 0;
    
    offset = _getJpegThumbnailOffset(dir, size);
    if(size == 0) {
        return OR_ERROR_NOT_FOUND;
    }
    Trace(DEBUG1) << "Jpeg offset: " << offset << "\n";

    uint32_t x = 0;
    uint32_t y = 0;
    ::or_data_type _type = OR_DATA_TYPE_JPEG;
    boost::scoped_ptr<IO::StreamClone> s(new IO::StreamClone(m_io, offset));
    boost::scoped_ptr<JfifContainer> jfif(new JfifContainer(s.get(), 0));
    if (jfif->getDimensions(x,y)) {
        Trace(DEBUG1) << "JPEG dimensions x=" << x 
                      << " y=" << y << "\n";
    }
    if(_type != OR_DATA_TYPE_NONE) {
        uint32_t dim = std::max(x, y);
        m_thumbLocations[dim] = ThumbDesc(x, y, _type, offset, size);
        list.push_back(dim);
    }
	
    return OR_ERROR_NONE;
}

uint32_t Rw2File::_getJpegThumbnailOffset(const IfdDir::Ref & dir, uint32_t & len)
{
    IfdEntry::Ref e = dir->getEntry(IFD::RW2_TAG_JPEG_FROM_RAW);
	if(!e) {
	    len = 0;
		Trace(DEBUG1) << "JpegFromRaw not found\n";
		return 0;
	}
    uint32_t offset = e->offset();
    len = e->count();
    return offset;
}


::or_error Rw2File::_getRawData(RawData & data, uint32_t /*options*/)
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
	// RW2 file
	got_it = _cfaIfd->getIntegerValue(IFD::RW2_TAG_STRIP_OFFSETS, offset);
	if(got_it) {
        byte_length = m_container->file()->filesize() - offset;
    }
    else {
        // RAW file alternative.
        got_it = _cfaIfd->getIntegerValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
        if(!got_it) {
            Trace(DEBUG1) << "offset not found\n";
            return OR_ERROR_NOT_FOUND;
        }
        got_it = _cfaIfd->getIntegerValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_length);
        if(!got_it) {
            Trace(DEBUG1) << "byte len not found\n";
            return OR_ERROR_NOT_FOUND;
        }
    }

	uint32_t x, y;
	x = 0;
	y = 0;
	got_it = _cfaIfd->getIntegerValue(IFD::RW2_TAG_SENSOR_WIDTH, x);
	if(!got_it) {
		Trace(DEBUG1) << "X not found\n";
		return OR_ERROR_NOT_FOUND;
	}
	got_it = _cfaIfd->getIntegerValue(IFD::RW2_TAG_SENSOR_HEIGHT, y);
	if(!got_it) {
		Trace(DEBUG1) << "Y not found\n";
		return OR_ERROR_NOT_FOUND;
	}
	
	// this is were things are complicated. The real size of the raw data
	// is whatever is read (if compressed) 
	void *p = data.allocData(byte_length);
	size_t real_size = m_container->fetchData(p, offset, 
											  byte_length);

	if (real_size / (x * 8 / 7) == y) {
		data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);
		data.setCompression(PANA_RAW_COMPRESSION);
	}
	else if (real_size < byte_length) {
		Trace(WARNING) << "Size mismatch for data: expected " << byte_length 
			<< " got " << real_size << " ignoring.\n";
		return OR_ERROR_NOT_FOUND;
	}
	else {
		data.setDataType(OR_DATA_TYPE_CFA);
	}
	data.setCfaPattern(OR_CFA_PATTERN_BGGR);
	
	
	// they are not all RGGB.
	// but I don't seem to see where this is encoded.
	// 
	data.setDimensions(x, y);

	Trace(DEBUG1) << "In size is " << data.width() 
				  << "x" << data.height() << "\n";
	// get the sensor info
	IfdEntry::Ref e = _cfaIfd->getEntry(IFD::RW2_TAG_SENSOR_LEFTBORDER);
	x = e->getIntegerArrayItem(0);
	e = _cfaIfd->getEntry(IFD::RW2_TAG_SENSOR_TOPBORDER);
	y = e->getIntegerArrayItem(0);
	e = _cfaIfd->getEntry(IFD::RW2_TAG_IMAGE_HEIGHT);
	uint32_t h = e->getIntegerArrayItem(0);
	e = _cfaIfd->getEntry(IFD::RW2_TAG_IMAGE_WIDTH);
	uint32_t w = e->getIntegerArrayItem(0);

	data.setRoi(x, y, w, h);

	return ret;
}


}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
