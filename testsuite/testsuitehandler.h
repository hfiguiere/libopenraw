/*
 * libopenraw - testsuitehandler.h
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



#ifndef _TEST_TESTSUITEHANDLER_H_
#define _TEST_TESTSUITEHANDLER_H_

#include "xmlhandler.h"
#include "testsuite.h"

class TestContext
	: public xml::Context
{
public:
	TestContext(const xml::HandlerPtr & handler, Test::Ptr test);

	xml::ContextPtr startElement(int32_t element);
	void endElement(int32_t element);
private:
	Test::Ptr m_test;
	bool m_results;
};


class TestSuiteHandler
	: public xml::Handler
{
public:
	TestSuiteHandler(const std::string & filename, TestSuite * ts);

	virtual xml::ContextPtr startElement(int32_t element);
private:
	TestSuite * m_ts;
};

#endif
