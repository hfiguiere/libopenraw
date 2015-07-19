/*
 * libopenraw - testsuitehandler.cpp
 *
 * Copyright (C) 2008-2015 Hubert Figuiere
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


#include <stdio.h>
#include <map>
#include <memory>

#include "testsuitehandler.h"
#include "testsuite.h"
#include "testsuitetags.h"

TestContext::TestContext(const xml::HandlerPtr & handler,
                         TestSuite * ts,
                         Test::Ptr&& test)
  : xml::Context(handler),
    m_ts(ts),
    m_test(std::move(test)),
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
	case XML_rawTypeId:
	case XML_thumbNum:
	case XML_thumbSizes:
	case XML_thumbFormats:
	case XML_thumbDataSizes:
	case XML_thumbMd5:
	case XML_rawDataType:
	case XML_rawDataSize:
	case XML_rawDataDimensions:
	case XML_rawDataRoi:
	case XML_rawCfaPattern:
	case XML_rawMinValue:
	case XML_rawMaxValue:
	case XML_rawMd5:
	case XML_rawDecompressedMd5:
	case XML_metaOrientation:
        case XML_exifMake:
        case XML_exifModel:
        case XML_makerNoteCount:
        case XML_makerNoteId:
		// other tests...
		if(m_results) {
			std::string & s(m_test->results()[element]);
			ctxt.reset(new xml::SimpleElementContext(m_handler, s));
		}
		break;
	default:
			fprintf(stderr, "Unhandled tag %d\n", element);
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
        case XML_test:
                m_ts->add_test(std::move(m_test));
                m_test.reset();
                break;
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
		ctxt.reset(new TestContext(m_handler, m_ts, Test::Ptr(new Test())));
		break;
	}
	default:
		break;
	}

	if(!ctxt)
		ctxt = shared_from_this();

	return ctxt;
}

void TestSuiteHandler::endElement(int32_t element)
{
    switch(element)
    {
    default:
        break;
    }
}
