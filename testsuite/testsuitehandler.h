


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
private:
	Test::Ptr m_test;
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
