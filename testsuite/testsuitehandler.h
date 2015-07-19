/* -*- Mode: C++ -*- */
/*
 * libopenraw - testsuitehandler.h
 *
 * Copyright (C) 2008-2015 Hubert Figuiere
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

#ifndef _TEST_TESTSUITEHANDLER_H_
#define _TEST_TESTSUITEHANDLER_H_

#include <stdint.h>
#include <string>

#include "xmlhandler.h"
#include "testsuite.h"

class TestContext : public xml::Context {
public:
    TestContext(const xml::HandlerPtr& handler, TestSuite* ts, Test::Ptr&& test);

    xml::ContextPtr startElement(int32_t element) override;
    void endElement(int32_t element) override;

private:
    TestSuite* m_ts;
    Test::Ptr m_test;
    bool m_results;
};

class TestSuiteHandler : public xml::Handler {
public:
    TestSuiteHandler(const std::string& filename, TestSuite* ts);

    virtual xml::ContextPtr startElement(int32_t element) override;
    virtual void endElement(int32_t element) override;

private:
    TestSuite* m_ts;
};

#endif
