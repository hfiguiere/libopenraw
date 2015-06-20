/*
 * libopenraw - xmlhandler.cpp
 *
 * Copyright (C) 2008 Hubert Figuiere
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


#include <string>

#include <boost/test/minimal.hpp>

#include "xmlhandler.h"

enum {
	XML_root = 1,
	XML_foo = 2,
	XML_bar = 3
};

static const xml::tag_map_definition_t tags[] = {
	{ "root", XML_root },
	{ "foo",  XML_foo },
	{ "bar",  XML_bar },
	{ 0,      0 }
};

class TestHandler 
	: public xml::Handler
{
public:
	TestHandler(const std::string & filename)
		: xml::Handler(filename)
		, rootFound(false)
		{
			mapTags(tags);
		}

	virtual xml::ContextPtr startElement(int32_t element) override
		{
			xml::ContextPtr ctx;

			switch(element) {
			case XML_root:
				rootFound = true;
				break;
			case XML_foo:
				ctx.reset(new xml::SimpleElementContext(std::static_pointer_cast<xml::Handler>(shared_from_this()),
														foo));
				break;
			case XML_bar:
				ctx.reset(new xml::SimpleElementContext(std::static_pointer_cast<xml::Handler>(shared_from_this()),
														bar));
				break;
			default:
				break;
			}
			if(!ctx) {
				ctx = shared_from_this();
			}
			return ctx;
		}

	bool        rootFound;
	std::string foo;
	std::string bar;
};

int test_main( int, char *[] )             // note the name!
{
	std::string dir;
	std::string filename;
	const char * pdir = getenv("srcdir");
	if(pdir == NULL) {
		dir = ".";
	}
	else {
		dir = pdir;
	}

	filename = dir;
	filename += "/test.xml";
	xml::HandlerPtr handler(new TestHandler(filename));
	BOOST_CHECK(handler->process());
	BOOST_CHECK(std::static_pointer_cast<TestHandler>(handler)->rootFound);
	BOOST_CHECK(std::static_pointer_cast<TestHandler>(handler)->foo == "foo");
	BOOST_CHECK(std::static_pointer_cast<TestHandler>(handler)->bar == "bar");

	return 0;
}


