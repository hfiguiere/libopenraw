/*
 * libopenraw - testsuitehandler.cpp
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





#include "testsuitehandler.h"
#include "testsuite.h"
#include "testsuitetags.h"

TestContext::TestContext(const xml::HandlerPtr & handler, Test::Ptr test)
	: xml::Context(handler),
	  m_test(test),
	  m_results(false)
{

}


xml::ContextPtr TestContext::startElement(int32_t element)
{
	xml::ContextPtr ctxt;

	switch(element)
	{
	case XML_name:
		ctxt.reset(new xml::SimpleElementContext(m_handler, m_test->name()));
		break;
	case XML_file:
		ctxt.reset(new xml::SimpleElementContext(m_handler, m_test->file()));
		break;
	case XML_source:
		ctxt.reset(new xml::SimpleElementContext(m_handler, m_test->source()));
		break;
	case XML_results:
		m_results = true;
		break;
	case XML_rawType:
	case XML_thumbNum:
	case XML_thumbSizes:
	case XML_thumbFormats:
	case XML_thumbDataSizes:
	case XML_rawDataType:
	case XML_rawDataSize:
	case XML_rawDataDimensions:
	case XML_rawCfaPattern:
	case XML_rawMd5:
	case XML_rawDecompressedMd5:
	case XML_metaOrientation:
		// other tests...
		if(m_results) {
			std::string & s(m_test->results()[element]);
			ctxt.reset(new xml::SimpleElementContext(m_handler, s));
		}
		break;
	default: 
		break;
	}

	if(!ctxt)
		ctxt = shared_from_this();

	return ctxt;
	
}


void TestContext::endElement(int32_t element)
{
	switch(element)
	{
	case XML_results:
		m_results = false;
		break;
	default:
		break;
	}
}



TestSuiteHandler::TestSuiteHandler(const std::string & filename, TestSuite * ts)
	: xml::Handler(filename),
	  m_ts(ts)
{
	mapTags(testsuitetags);
}


xml::ContextPtr TestSuiteHandler::startElement(int32_t element)
{
	xml::ContextPtr ctxt;

	switch(element)
	{
	case XML_testsuite:
		break;
	case XML_test:
	{
		Test::Ptr newtest(new Test());
		m_ts->add_test(newtest);
		ctxt.reset(new TestContext(m_handler, newtest));
		break;
	}
	default: 
		break;
	}

	if(!ctxt)
		ctxt = shared_from_this();

	return ctxt;
}

