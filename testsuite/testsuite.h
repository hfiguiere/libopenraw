


#ifndef _TEST_TESTSUITE_H_
#define _TEST_TESTSUITE_H_

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

class Test
{
public:
	typedef boost::shared_ptr<Test> Ptr;

	Test();
	
	std::string & name()
		{ return m_name; }
	std::string & file()
		{ return m_file; }
	std::string & source()
		{ return m_source; }
	/** return 0 the test ran perfectly */
	int run();
private:
	std::string m_name;
	std::string m_file;
	std::string m_source;
};

class TestSuite
{
public:
	TestSuite();
	
	int load_tests(const char * testsuite_file);
	/** return 0 if all test ran perfectly */
	int run_all();
	/** add a test. own the test */
	void add_test(Test::Ptr t);
private:

	std::vector<Test::Ptr> m_tests;
};



#endif
