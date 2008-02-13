/*
 * libopenraw - testsuite.cpp
 *
 * Copyright (C) 2008 Hubert Figuiere
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


#ifndef _TEST_TESTSUITE_H_
#define _TEST_TESTSUITE_H_

#include <vector>
#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

namespace OpenRaw {
	class RawFile;
	class RawData;
}

class Test
{
public:
	typedef boost::shared_ptr<Test> Ptr;

	Test();
	~Test();
	
	std::string & name()
		{ return m_name; }
	std::string & file()
		{ return m_file; }
	std::string & source()
		{ return m_source; }
	std::map<int, std::string> & results()
		{ return m_results; }
	/** return 0 the test ran perfectly */
	int run();
private:

	bool testRawType(const std::string & result);
	bool testThumbNum(const std::string & result);
	bool testThumbSizes(const std::string & result);
	bool testThumbFormats(const std::string & result);
	bool testThumbDataSizes(const std::string & result);
	bool testRawDataType(const std::string & result);
	bool testRawDataSize(const std::string & result);
	bool testRawDataDimensions(const std::string & result);
	bool testRawCfaPattern(const std::string & result);
	bool testRawMd5(const std::string & result);
	bool testRawDecompressedMd5(const std::string & result);
	bool testMetaOrientation(const std::string & result);

	std::string m_name;
	std::string m_file;
	std::string m_source;
	std::map<int, std::string> m_results;
	OpenRaw::RawFile *m_rawfile;
	OpenRaw::RawData * m_rawdata;
	int m_total, m_success, m_failure;
};

class TestSuite
{
public:
	TestSuite();
	
	int load_tests(const char * testsuite_file);
	/** return 0 if all test ran perfectly */
	int run_all();
	/** add a test. own the test */
	void add_test(const Test::Ptr & t);
private:

	std::vector<Test::Ptr> m_tests;
};



#endif
