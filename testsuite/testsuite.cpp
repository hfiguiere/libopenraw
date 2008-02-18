/*
 * libopenraw - testsuite.cpp
 *
 * Copyright (C) 2008 Hubert Figuiere
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


#include <stdlib.h>
#include <stdio.h>

#include <libxml/xmlreader.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <vector>
#include <stack>
#include <numeric>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal

#include <libopenraw++/rawfile.h>
#include <libopenraw++/rawdata.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/bitmapdata.h>

#include "xmlhandler.h"
#include "testsuite.h"
#include "testsuitehandler.h"
#include "testsuitetags.h"

using OpenRaw::RawFile;
using OpenRaw::BitmapData;
using OpenRaw::RawData;
using OpenRaw::Thumbnail;


#define RETURN_TEST_EQUALS(a,b,expected)			\
	{											\
		bool _success = (a == b);					\
		if(!_success) {											\
			fprintf(stderr, "FAILED: %s on equality. found %d , " \
					"expected '%s'\n",	\
					__FUNCTION__, a, expected.c_str());		\
		}														\
		return _success;										\
	}


#define RETURN_TEST(test,expected)				\
	{											\
		bool _success = (test);					\
		if(!_success) {											\
			fprintf(stderr, "FAILED: %s on '%s', expected '%s'\n",	\
					__FUNCTION__, #test, expected.c_str());		\
		}														\
		return _success;										\
	}

#define RETURN_FAIL(message,expected)			\
	{											\
		fprintf(stderr, "FAILED: %s with '%s', expected '%s'\n",	\
				__FUNCTION__, message, expected.c_str());		\
		return false;											\
	}

namespace {
	bool equalCfaPattern(const std::string & result, RawData::CfaPattern t)
	{
		bool equal = false;
		switch(t) {
		case OR_CFA_PATTERN_NONE:
			equal = (result == "NONE");
			break;
		case OR_CFA_PATTERN_NON_RGB22:
			equal = (result == "NON_RGB22");
			break;
		case OR_CFA_PATTERN_RGGB:
			equal = (result == "RGGB");
			break;
		case OR_CFA_PATTERN_GBRG:
			equal = (result == "GBRG");
			break;
		case OR_CFA_PATTERN_BGGR:
			equal = (result == "BGGR");
			break;
		case OR_CFA_PATTERN_GRBG:
			equal = (result == "GRBG");
			break;
		default:
			break;
		}
		return equal;
	}
	

	bool equalDataType(const std::string & result, BitmapData::DataType t)
	{
		bool equal = false;
		switch(t) {
		case OR_DATA_TYPE_PIXMAP_8RGB:
			equal = (result == "8RGB");
			break;
		case OR_DATA_TYPE_JPEG:
			equal = (result == "JPEG");
			break;
		case OR_DATA_TYPE_TIFF:
			equal = (result == "TIFF");
			break;
		case OR_DATA_TYPE_PNG:
			equal = (result == "PNG");
			break;
		case OR_DATA_TYPE_CFA:
			equal = (result == "CFA");
			break;
		case OR_DATA_TYPE_COMPRESSED_CFA:
			equal = (result == "COMP_CFA");
			break;
		default:
			break;
		}
		return equal;
	}

}

Test::Test()
	: m_rawfile(NULL),
	  m_rawdata(NULL),
	  m_total(0), m_success(0), m_failure(0)
{
}

Test::~Test()
{
	delete m_rawfile;
	delete m_rawdata;
}

bool Test::testRawType(const std::string & result)
{
	RawFile::Type t = m_rawfile->type();
	switch(t) {
	case OR_RAWFILE_TYPE_CR2:
		RETURN_TEST(result == "CR2", result);
		break;
	case OR_RAWFILE_TYPE_CRW:
		RETURN_TEST(result == "CRW", result);
		break;
	case OR_RAWFILE_TYPE_NEF:
		RETURN_TEST(result == "NEF", result);
		break;
	case OR_RAWFILE_TYPE_MRW:
		RETURN_TEST(result == "MRW", result);
		break;
	case OR_RAWFILE_TYPE_ARW:
		RETURN_TEST(result == "ARW", result);
		break;
	case OR_RAWFILE_TYPE_DNG:
		RETURN_TEST(result == "DNG", result);
		break;
	case OR_RAWFILE_TYPE_ORF:
		RETURN_TEST(result == "ORF", result);
		break;
	case OR_RAWFILE_TYPE_PEF:
		RETURN_TEST(result == "PEF", result);
		break;
	case OR_RAWFILE_TYPE_ERF:
		RETURN_TEST(result == "ERF", result);
		break;
	default:
		break;
	}
	RETURN_TEST(false, result);
}


bool Test::testThumbNum(const std::string & result)
{
	const std::vector<uint32_t> & thumbs = m_rawfile->listThumbnailSizes();
	int num = thumbs.size();
	try {
		RETURN_TEST(num == boost::lexical_cast<int>(result), result);
	}
	catch(...)
	{
	}
	RETURN_FAIL("conversion failed", result);
}

bool Test::testThumbSizes(const std::string & result)
{
	std::vector<uint32_t> thumbs = m_rawfile->listThumbnailSizes();
	std::vector< std::string > v;
	boost::split(v, result, boost::is_any_of(" "));
	if(v.size() != thumbs.size()) {
		RETURN_FAIL("mismatch number of elements", result);
	}
	std::vector<uint32_t> v2;
	for(std::vector< std::string >::iterator iter = v.begin();
		iter != v.end(); iter++) 
	{
		try {
			v2.push_back(boost::lexical_cast<uint32_t>(*iter));
		}
		catch(...)
		{
			RETURN_FAIL("conversion failed", result);
		}
	}
	RETURN_TEST(std::equal(thumbs.begin(), thumbs.end(), v2.begin()), result);
}

bool Test::testThumbFormats(const std::string & result)
{
	bool success = true;
	std::vector<uint32_t> thumbs = m_rawfile->listThumbnailSizes();
	std::vector< std::string > v;
	boost::split(v, result, boost::is_any_of(" "));
	std::vector< std::string >::iterator result_iter = v.begin();
	if(v.size() != thumbs.size()) {
		RETURN_FAIL("mismatch number of elements", result);
	}
	for(std::vector<uint32_t>::iterator thumbs_iter = thumbs.begin();
		thumbs_iter != thumbs.end(); thumbs_iter++, result_iter++) 
	{
		Thumbnail t;
		m_rawfile->getThumbnail(*thumbs_iter, t);
		success &= equalDataType(*result_iter, t.dataType());
	}
	RETURN_TEST(success, result);
}

bool Test::testThumbDataSizes(const std::string & result)
{
	bool success = true;
	std::vector<uint32_t> thumbs = m_rawfile->listThumbnailSizes();
	std::vector< std::string > v;
	boost::split(v, result, boost::is_any_of(" "));
	std::vector< std::string >::iterator result_iter = v.begin();
	if(v.size() != thumbs.size()) {
		RETURN_FAIL("mismatch number of elements", result);
	}
	for(std::vector<uint32_t>::iterator thumbs_iter = thumbs.begin();
		thumbs_iter != thumbs.end(); thumbs_iter++, result_iter++) 
	{
		Thumbnail t;
		m_rawfile->getThumbnail(*thumbs_iter, t);
		try {
			success &= (boost::lexical_cast<uint32_t>(*result_iter) == t.size());
		}
		catch(...) {
			RETURN_FAIL("conversion failed", result);
		}
	}
	RETURN_TEST(success, result);
}

namespace {
	RawData * loadRawData(RawFile * file)
	{
		RawData *rawdata = new RawData();
		::or_error err;
		err = file->getRawData(*rawdata, OR_OPTIONS_NONE);
		if(OR_ERROR_NONE != err) {
			delete rawdata; 
			rawdata = NULL;
		}
		return rawdata;
	}

	uint32_t computeCrc(const RawData * rawdata)
	{
		boost::crc_optimal<8, 0x1021, 0xFFFF, 0, false, false>  crc_ccitt2;
		
		const uint8_t * data = static_cast<uint8_t *>(rawdata->data());
		size_t data_len = rawdata->size();
		crc_ccitt2 = std::for_each( data, data + data_len, crc_ccitt2 );
		return crc_ccitt2();
	}

}

bool Test::testRawDataType(const std::string & result)
{
	if(m_rawdata == NULL) {
		m_rawdata = loadRawData(m_rawfile);
		if(m_rawdata == NULL) {
			RETURN_FAIL("failed to get rawData", result);
		}
	}
	RETURN_TEST(equalDataType(result, m_rawdata->dataType()), result);
}


bool Test::testRawDataSize(const std::string & result)
{
	if(m_rawdata == NULL) {
		m_rawdata = loadRawData(m_rawfile);
		if(m_rawdata == NULL) {
			RETURN_FAIL("failed to get rawData", result);
		}
	}
	try {
		RETURN_TEST(boost::lexical_cast<uint32_t>(result) == m_rawdata->size(),
					result);
	}
	catch(...) {
	}
	RETURN_FAIL("conversion failed", result);		
}

bool Test::testRawDataDimensions(const std::string & result)
{
	if(m_rawdata == NULL) {
		m_rawdata = loadRawData(m_rawfile);
		if(m_rawdata == NULL) {
			RETURN_FAIL("failed to get rawData", result);
		}
	}
	std::vector< std::string > v;
	boost::split(v, result, boost::is_any_of(" "));
	if(v.size() != 2) {
		RETURN_FAIL("mismatch number of elements from expected result", result);
	}
	uint32_t x, y;
	try {
		x = boost::lexical_cast<uint32_t>(v[0]);
		y = boost::lexical_cast<uint32_t>(v[1]);
	}
	catch(...)
	{
		RETURN_FAIL("conversion failed", result);
	}
	RETURN_TEST(x == m_rawdata->x() && y == m_rawdata->y(), result)
}


bool Test::testRawCfaPattern(const std::string & result)
{
	if(m_rawdata == NULL) {
		m_rawdata = loadRawData(m_rawfile);
		if(m_rawdata == NULL) {
			RETURN_FAIL("failed to get rawData", result);
		}
	}
	RETURN_TEST(equalCfaPattern(result, m_rawdata->cfaPattern()), result);
}

bool Test::testRawMd5(const std::string & result)
{
	if(m_rawdata == NULL) {
		m_rawdata = loadRawData(m_rawfile);
		if(m_rawdata == NULL) {
			RETURN_FAIL("failed to get rawData", result);
		}
	}

	uint32_t crc = computeCrc(m_rawdata);

	uint32_t expected = 0;
	try { 
		expected = boost::lexical_cast<uint32_t>(result);
	}
	catch(...)
	{
		RETURN_FAIL("conversion failed", result);
	}
	RETURN_TEST_EQUALS(crc, expected, result);
}


bool Test::testRawDecompressedMd5(const std::string & result)
{
	if(m_rawdata == NULL) {
		m_rawdata = loadRawData(m_rawfile);
		if(m_rawdata == NULL) {
			RETURN_FAIL("failed to get rawData", result);
		}
	}
	return false;
}


bool Test::testMetaOrientation(const std::string & result)
{
	int32_t orientation = m_rawfile->getOrientation();
	RETURN_TEST_EQUALS(orientation, boost::lexical_cast<int32_t>(result), result);
}


/** run the test.
 * @return the number of failures. 0 means success
 */
int Test::run()
{
	// load rawfile
	fprintf(stderr, "running test %s on file %s\n", m_name.c_str(),
			m_file.c_str());

	struct stat buf;
	if(stat(m_file.c_str(), &buf) == -1) {
		fprintf(stderr, "File not found, skipping. (%d)\n", errno);
		return 0;
	}
	m_rawfile = RawFile::newRawFile(m_file.c_str());

	if(m_rawfile == NULL) {
		RETURN_FAIL("m_rawfile == NULL", std::string("not NULL"));
	}
	
	std::map<int, std::string>::const_iterator iter;
	for(iter = m_results.begin(); iter != m_results.end(); iter++) {
		bool pass = false;
		switch(iter->first)
		{
		case XML_rawType:
			pass = testRawType(iter->second);
			break;
		case XML_thumbNum:
			pass = testThumbNum(iter->second);
			break;
		case XML_thumbSizes:
			pass = testThumbSizes(iter->second);
			break;
		case XML_thumbFormats:
			pass = testThumbFormats(iter->second);
			break;
		case XML_thumbDataSizes:
			pass = testThumbDataSizes(iter->second);
			break;
		case XML_rawDataType:
			pass = testRawDataType(iter->second);
			break;
		case XML_rawDataSize:
			pass = testRawDataSize(iter->second);
			break;
		case XML_rawDataDimensions:
			pass = testRawDataDimensions(iter->second);
			break;
		case XML_rawCfaPattern:
			pass = testRawCfaPattern(iter->second);
			break;
		case XML_rawMd5:
			pass = testRawMd5(iter->second);
			break;
		case XML_rawDecompressedMd5:
			pass = testRawDecompressedMd5(iter->second);
			break;
		case XML_metaOrientation:
			pass = testMetaOrientation(iter->second);
			break;
		default:
			pass = false;
			break;
		}
		m_total++;
		if(!pass) {
			m_failure++;
		}
		else {
			m_success++;
		}
	}
	fprintf(stderr, "total %d, success %d, failure %d\n", m_total,
		    m_success, m_failure);
	return m_failure;
}


TestSuite::TestSuite()
{
}


void TestSuite::add_test(const Test::Ptr & t)
{
	m_tests.push_back(t);
}


int TestSuite::load_tests(const char * testsuite_file)
{
	xml::HandlerPtr handler(new TestSuiteHandler(testsuite_file, this));
	
	bool has_data = false;

	has_data = handler->process();

	return !has_data;
}

int TestSuite::run_all()
{
	int failures = 0;
	std::vector<Test::Ptr>::iterator iter(m_tests.begin());
	for( ; iter != m_tests.end(); ++iter) {
		failures += (*iter)->run();
	}
	return failures;
}

int main(int /*argc*/, char ** argv)
{
	const char * srcdir = getenv("srcdir");
	if(srcdir == NULL) {
		srcdir = "./";
	}
	std::string testsuite_file = srcdir;
	testsuite_file += "/";
	testsuite_file += (argv[1] ? argv[1] : "testsuite.xml");

	or_debug_set_level(ERROR);

	TestSuite testsuite;
	testsuite.load_tests(testsuite_file.c_str());
	return testsuite.run_all();
}


