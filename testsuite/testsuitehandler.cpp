




#include "testsuitehandler.h"
#include "testsuite.h"
#include "testsuitetags.h"

TestContext::TestContext(const xml::HandlerPtr & handler, Test::Ptr test)
	: xml::Context(handler),
	  m_test(test)
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
	default: 
		break;
	}

	if(!ctxt)
		ctxt = shared_from_this();

	return ctxt;
	
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

