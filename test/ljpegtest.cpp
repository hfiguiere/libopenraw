/*
 * Copyright (C) 2007 Hubert Figuiere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */



#include <string>

#include <boost/test/auto_unit_test.hpp>

#include <libopenraw++/rawdata.h>

#include "io/file.h"
#include "rawcontainer.h"
#include "jfifcontainer.h"
#include "ljpegdecompressor.h"
#include "ljpegdecompressor_priv.h"

using boost::unit_test::test_suite;
using OpenRaw::RawData;
using OpenRaw::IO::File;

std::string g_testfile;

namespace OpenRaw { namespace Internals {
void test_ljpeg2()
{
	RawData *decompData;
	File *stream = new File(g_testfile.c_str());
	RawContainer *container = new JFIFContainer(stream, 0);

	LJpegDecompressor decompressor(stream, container);

	DecompressInfo dcInfo;

	decompressor.ReadFileHeader(&dcInfo); 
	decompressor.ReadScanHeader(&dcInfo);

	decompressor.m_output = new RawData();
	RawData * bitmap = decompressor.m_output;

	uint32_t bpc = dcInfo.dataPrecision;
	BOOST_CHECK_EQUAL(bpc, (uint32_t)16);

	bitmap->setDataType(OR_DATA_TYPE_CFA);
	bitmap->setBpc(bpc);
	bitmap->allocData(dcInfo.imageWidth
										* sizeof(uint16_t) 
										* dcInfo.imageHeight
										* dcInfo.numComponents);

	uint32_t width = (dcInfo.imageWidth * dcInfo.numComponents);
	bitmap->setDimensions(width, dcInfo.imageHeight);

	decompressor.DecoderStructInit(&dcInfo);
	decompressor.HuffDecoderInit(&dcInfo);
//	decompressor.DecodeImage(&dcInfo);

	JpegComponentInfo *compptr;
	HuffmanTable *dctbl;
	uint16_t ci = dcInfo.MCUmembership[0];
	BOOST_CHECK_EQUAL(ci, 0);
	compptr = dcInfo.curCompInfo[ci];
	dctbl = dcInfo.dcHuffTblPtrs[compptr->dcTblNo];
	int32_t d, s;
	/*
	 * Section F.2.2.1: decode the difference
	 */
	s = decompressor.HuffDecode (dctbl);
	if (s) {
		d = decompressor.get_bits(s);
		BOOST_CHECK_EQUAL(d, 230);
		decompressor.HuffExtend(d,s);
		BOOST_CHECK_EQUAL(d, -32537);
	} else {
		d = 0;
	}

	delete decompressor.m_output;
	delete decompData;
	delete container;
	delete stream;
}

} }


using namespace OpenRaw::Internals;

void test_ljpeg()
{
	RawData *decompData;
	File *s = new File(g_testfile.c_str());
	RawContainer *container = new JFIFContainer(s, 0);

	LJpegDecompressor decompressor(s, container);

	decompData = decompressor.decompress();

	delete decompData;
	delete container;
	delete s;
}



test_suite*
init_unit_test_suite( int argc, char * argv[] ) 
{
	test_suite* test = BOOST_TEST_SUITE("test ljpeg");
	
	if (argc == 1) {
		// no argument, lets run like we are in "check"
		const char * srcdir = getenv("srcdir");
		
		BOOST_ASSERT(srcdir != NULL);
		g_testfile = std::string(srcdir);
		g_testfile += "/ljpegtest1.jpg";
	}
	else {
		g_testfile = argv[1];
	}
	
	test->add(BOOST_TEST_CASE(&test_ljpeg));
	test->add(BOOST_TEST_CASE(&OpenRaw::Internals::test_ljpeg2));

	return test;
}
