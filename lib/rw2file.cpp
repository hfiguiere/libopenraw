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
#include <libopenraw/cameraids.h>
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
#include "rawfile_private.h"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

#define OR_MAKE_PANASONIC_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,camid)
#define OR_MAKE_LEICA_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF1), 15, 0xf92,
    { 7888,-1902,-1011,-8106,16085,2099,-2353,2866,7330 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF2), 143, 0xfff,
    { 7888,-1902,-1011,-8106,16085,2099,-2353,2866,7330 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF3), 143, 0xfff,
    { 9051,-2468,-1204,-5212,13276,2121,-1197,2510,6890 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX1), 143, 0,
    { 6763,-1919,-863,-3868,11515,2684,-1216,2387,5879 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ8), 0, 0xf7f,
    { 8986,-2755,-802,-6341,13575,3077,-1476,2144,6379 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ18), 0, 0,
    { 9932,-3060,-935,-5809,13331,2753,-1267,2155,5575 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ28), 15, 0xf96,
    { 10109,-3488,-993,-5412,12812,2916,-1305,2140,5543 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ30), 0, 0xf94,
    { 10976,-4029,-1141,-7918,15491,2600,-1670,2071,8246 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ50), 0, 0,
    { 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ100), 143, 0xfff,
    { 16197,-6146,-1761,-2393,10765,1869,366,2238,5248 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G1), 15, 0xf94,
    { 8199,-2065,-1056,-8124,16156,2033,-2458,3022,7220 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G2), 15, 0xf3c,
    { 10113,-3400,-1114,-4765,12683,2317,-377,1437,6710 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G3), 143, 0xfff,
    { 6763,-1919,-863,-3868,11515,2684,-1216,2387,5879 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G10), 0, 0,
    { 10113,-3400,-1114,-4765,12683,2317,-377,1437,6710 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH1), 15, 0xf92,
    { 6299,-1466,-532,-6535,13852,2969,-2331,3112,5984 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH2), 15, 0xf95,
    { 7780,-2410,-806,-3913,11724,2484,-1018,2390,5298 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX2), 0, 0,
    { 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX3), 15, 0,
    { 8128,-2668,-655,-6134,13307,3161,-1782,2568,6083 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX5), 143, 0,
    { 10909,-4295,-948,-1333,9306,2399,22,1738,4582 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L1), 0, 0xf7f,
    { 8054,-1885,-1025,-8349,16367,2040,-2805,3542,7629 } },
  { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L10), 15, 0xf96,
    { 8025,-1942,-1050,-7920,15904,2100,-2456,3005,7039 } },

  { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DIGILUX2), 0, 0,
    { 11340,-4069,-1275,-7555,15266,2448,-2960,3426,7685 } },
  { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_3), 0, 0,
    { 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
  { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_1), 0, 0,
    { 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },

  { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t Rw2File::s_def[] = {
	{ "DMC-GF1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF1) },
	{ "DMC-GF2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF2) },
	{ "DMC-GF3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF3) },
	{ "DMC-GF5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF5) },
	{ "DMC-GX1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX1) },
	{ "DMC-FZ8", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ8) },
	{ "DMC-FZ18", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ18) },
	{ "DMC-FZ28", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ28) },
	{ "DMC-FZ30", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ30) },
	{ "DMC-FZ50", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ50) },
	{ "DMC-FZ100", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ100) },
	{ "DMC-FZ200", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ200) },
	{ "DMC-G1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G1) },
	{ "DMC-G2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G2) },
	{ "DMC-G3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G3) },
	{ "DMC-G5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G5) },
	{ "DMC-G10", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G10) },
	{ "DMC-GH1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH1) },
	{ "DMC-GH2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH2) },
	{ "DMC-GH3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH3) },
	{ "DMC-LX2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX2) },
	{ "DMC-LX3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX3) },
	{ "DMC-LX5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX5) },
	{ "DMC-LX7", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX7) },
	{ "DMC-L1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L1) },
	{ "DMC-L10", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L10) },

	{ "DIGILUX 2", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DIGILUX2) },
	{ "D-LUX 3", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_3) },
	{ "V-LUX 1", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_1) },

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
  _setMatrices(s_matrices);
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
        _addThumbnail(dim, ThumbDesc(x, y, _type, offset, size));
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
		data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);
		data.setCompression(PANA_RAW_COMPRESSION);
	}
	else if (real_size < byte_length) {
		Trace(WARNING) << "Size mismatch for data: expected " << byte_length 
			<< " got " << real_size << " ignoring.\n";
		return OR_ERROR_NOT_FOUND;
	}
	else {
		data.setDataType(OR_DATA_TYPE_RAW);
	}
	data.setCfaPatternType(OR_CFA_PATTERN_BGGR);
	
	
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
