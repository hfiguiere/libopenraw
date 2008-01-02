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



#include <stdlib.h>
#include <stdio.h>

#include <libxml/xmlreader.h>

#include <string>
#include <vector>
#include <stack>
#include <boost/shared_ptr.hpp>

#include "xmlhandler.h"
#include "testsuite.h"
#include "testsuitehandler.h"

Test::Test()
{
}


TestSuite::TestSuite()
{
}


void TestSuite::add_test(Test::Ptr t)
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

int main(int argc, char ** argv)
{
	const char * srcdir = getenv("srcdir");
	if(srcdir == NULL) {
		srcdir = "./";
	}
	std::string testsuite_file = srcdir;
	testsuite_file += "/";
	testsuite_file += (argv[1] ? argv[1] : "testsuite.xml");

	TestSuite testsuite;
	testsuite.load_tests(testsuite_file.c_str());


	return 0;
}


