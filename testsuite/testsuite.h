/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */
/*
 * libopenraw - testsuite.h
 *
 * Copyright (C) 2008-2024 Hubert Figuière
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

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#if HAVE_CURL
#include <curl/curl.h>
#endif

#include <libxml/tree.h>

#include <string>
#include <set>
#include <map>
#include <memory>

#include <libopenraw/libopenraw.h>

struct ThumbnailDeleter
{
    void operator()(ORThumbnailRef p) const {
        or_thumbnail_release(p);
    }
};

struct RawDataDeleter
{
    void operator()(ORRawDataRef p) const {
        or_rawdata_release(p);
    }
};

struct RawFileDeleter
{
    void operator()(ORRawFileRef p) const {
        or_rawfile_release(p);
    }
};

class Test {
public:
    typedef std::unique_ptr<Test> Ptr;

    Test();
    ~Test();

    std::string& name() { return m_name; }
    std::string& file() { return m_file; }
    std::string& source() { return m_source; }
    bool download_disabled() const
        { return m_download_disabled; }
    bool& download_disabled()
        { return m_download_disabled; }
    std::string& referer()
        { return m_referer; }
    const std::string& referer() const
        { return m_referer; }
    std::map<int, std::string>& results() { return m_results; }
    std::set<int>& to_run() { return m_to_run; }
    /** return 0 the test ran perfectly */
    int run();

    /** a test in another test, only taking new values */
    void merge(const Test::Ptr& t);

private:
    bool testRawType(const std::string& result);
    bool testRawTypeId(const std::string& result);
    bool testThumbNum(const std::string& result);
    bool testThumbSizes(const std::string& result);
    bool testThumbFormats(const std::string& result);
    bool testThumbDataSizes(const std::string& result);
    bool testThumbMd5(const std::string& result);
    bool testRawDataType(const std::string& result);
    bool testRawDataSize(const std::string& result);
    bool testRawDataDimensions(const std::string& result);
    bool testRawDataActiveArea(const std::string& result);
    bool testRawDataUserCrop(const std::string& result);
    bool testRawDataUserAspectRatio(const std::string& result);
    bool testRawCfaPattern(const std::string& result);
    bool testRawMinValue(const std::string& result);
    bool testRawMaxValue(const std::string& result);
    bool testRawAsShotNeutral(const std::string& result);
    bool testRawMd5(const std::string& result);
    bool testRawDecompressedMd5(const std::string& result);
    bool testMetaOrientation(const std::string& result);
    bool testExifString(const char* meta_index, const std::string& results);
    bool testMakerNoteId(const std::string& result);
    bool testMakerNoteCount(const std::string& result);

    std::string m_name;
    std::string m_file;
    std::string m_source;
    bool m_download_disabled;
    std::string m_referer;
    std::map<int, std::string> m_results;
    std::set<int> m_to_run;
    // runtime data
    std::unique_ptr<_RawFile, RawFileDeleter> m_rawfile;
    std::unique_ptr<_RawData, RawDataDeleter> m_rawdata;
    int m_total, m_success, m_failure;
};

class TestSuite {
public:
    TestSuite();

    int load_tests(const char* testsuite_file);
    int load_overrides(const std::string& overrides_file);
    int bootstrap(const std::string& overrides_file,
                  const std::string& download_dir);
    /** return 0 if all test ran perfectly */
    int run_all();
    /** add a test. own the test */
    void add_test(Test::Ptr&& t);

private:
#if HAVE_CURL
    void walk_tests(xmlNode* test, CURL* handle,
                    const std::string& download_dir);
#endif
    std::map<std::string, Test::Ptr> m_tests;

    TestSuite(const TestSuite&) = delete;
    TestSuite & operator=(const TestSuite&) = delete;
};
