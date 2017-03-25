/*
 * libopenraw - testsuite.cpp
 *
 * Copyright (C) 2008-2016 Hubert Figuiere
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdint>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#include <libxml/parser.h>
#include <libxml/xmlstring.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal

#define IN_TESTSUITE
#include <libopenraw/debug.h>
#include <libopenraw/consts.h>
#include <libopenraw/metadata.h>

#include "rawfile.hpp"
#include "rawdata.hpp"
#include "thumbnail.hpp"
#include "bitmapdata.hpp"
#include "metavalue.hpp"
#include "cfapattern.hpp"

#include "xmlhandler.h"
#include "testsuite.h"
#include "testsuitehandler.h"
#include "testsuitetags.h"

// Internal stuff. Because we can.
#include "io/file.hpp"
#include "ifdfile.hpp"
#include "ifddir.hpp"
#include "makernotedir.hpp"

using OpenRaw::RawFile;
using OpenRaw::BitmapData;
using OpenRaw::RawData;
using OpenRaw::Thumbnail;

using std::unique_ptr;

#define RETURN_TEST_EQUALS(a,b) \
    {                               \
        bool _success = (a == b);   \
        if(!_success) {             \
            fprintf(stderr, "FAILED: %s on equality. found '%s', "  \
                    "expected '%s'\n",                       \
                    __FUNCTION__, a.c_str(), b.c_str());     \
        }                           \
        return _success;            \
    }

#define RETURN_TEST_EQUALS_N(a,b) \
    {                               \
        bool _success = (a == b);   \
        if(!_success) {             \
            fprintf(stderr, "FAILED: %s on equality. found %ld, "  \
                    "expected '%ld'\n",                             \
                    __FUNCTION__, (long)a, (long)b);                \
        }                           \
        return _success;            \
    }


// a and b are strings. b is the expected value
// success is return. Set to true if it is successful
#define CHECK_TEST_EQUALS(a,b,success)  \
    {                                   \
        success = (a == b);             \
        if(!success) {                  \
            fprintf(stderr, "FAILED: %s on equality with '%s', expected '%s'\n", \
                    __FUNCTION__, a.c_str(), b.c_str());     \
        }                                                    \
    }

// a and b are integers. b is the expected value
// success is return. Set to true if it is successful
#define CHECK_TEST_EQUALS_N(a,b,success) \
    {                                    \
        success = (a == b);              \
        if(!success) {                   \
            fprintf(stderr, "FAILED: %s on equality with %ld, expected %ld\n",	\
                    __FUNCTION__, (long)a, (long)b);   \
        }                                \
    }

#define RETURN_TEST(test,expected)       \
    {                                    \
        bool _success = (test);          \
        if(!_success) {                  \
            fprintf(stderr, "FAILED: %s on '%s', expected '%s'\n", \
                    __FUNCTION__, #test, expected.c_str()); \
        }                                \
        return _success;                 \
    }

#define RETURN_FAIL(message,expected)    \
    {                                    \
        fprintf(stderr, "FAILED: %s with '%s', expected '%s'\n", \
                __FUNCTION__, message, expected.c_str()); \
        return false;                    \
    }

namespace {
std::string cfaPatternToString(::or_cfa_pattern t)
{
    switch(t) {
    case OR_CFA_PATTERN_NONE:
        return "NONE";

    case OR_CFA_PATTERN_NON_RGB22:
        return "NON_RGB22";

    case OR_CFA_PATTERN_RGGB:
        return "RGGB";

    case OR_CFA_PATTERN_GBRG:
        return "GBRG";

    case OR_CFA_PATTERN_BGGR:
        return "BGGR";

    case OR_CFA_PATTERN_GRBG:
        return "GRBG";

    default:
        break;
    }
    return "";
}


bool equalDataType(const std::string & result, BitmapData::DataType t)
{
    bool equal = false;
    switch(t) {
    case OR_DATA_TYPE_PIXMAP_8RGB:
        equal = (result == "8RGB");
        break;
    case OR_DATA_TYPE_JPEG:
        equal = (result == "JPEG");
        break;
    case OR_DATA_TYPE_TIFF:
        equal = (result == "TIFF");
        break;
    case OR_DATA_TYPE_PNG:
        equal = (result == "PNG");
        break;
    case OR_DATA_TYPE_RAW:
        equal = (result == "RAW");
        break;
    case OR_DATA_TYPE_COMPRESSED_RAW:
        equal = (result == "COMP_RAW");
        break;
    default:
        break;
    }
    return equal;
}

}

Test::Test()
    : m_rawfile(nullptr),
      m_rawdata(nullptr),
      m_total(0), m_success(0), m_failure(0)
{
}

Test::~Test()
{
}

bool Test::testRawType(const std::string & result)
{
    RawFile::Type t = m_rawfile->type();

    // test the detection by content....
    RawFile::Type t2;
    OpenRaw::IO::File f(m_file.c_str());
    ::or_error err = f.open();
    if(err != OR_ERROR_NONE) {
        RETURN_FAIL("failed to open",
                    boost::lexical_cast<std::string>(err));
    }
    off_t len = f.filesize();
    unique_ptr<uint8_t[]> buff(new uint8_t[len]);
    int res = f.read(buff.get(), len);
    if(res == len) {
        unique_ptr<RawFile> r2(RawFile::newRawFileFromMemory(buff.get(), len));
        if(!r2) {
            RETURN_FAIL("failed to load from memory", std::string());
        }
        t2 = r2->type();
        if(t2 != t) {
            RETURN_FAIL("type mismatch", result);
        }
    }
    else {
        RETURN_FAIL("failed to load into buffer",
                    boost::lexical_cast<std::string>(res));
    }

    switch(t) {
    case OR_RAWFILE_TYPE_CR2:
        RETURN_TEST(result == "CR2", result);
        break;
    case OR_RAWFILE_TYPE_CRW:
        RETURN_TEST(result == "CRW", result);
        break;
    case OR_RAWFILE_TYPE_NEF:
        RETURN_TEST(result == "NEF", result);
        break;
    case OR_RAWFILE_TYPE_MRW:
        RETURN_TEST(result == "MRW", result);
        break;
    case OR_RAWFILE_TYPE_ARW:
        RETURN_TEST(result == "ARW", result);
        break;
    case OR_RAWFILE_TYPE_DNG:
        RETURN_TEST(result == "DNG", result);
        break;
    case OR_RAWFILE_TYPE_ORF:
        RETURN_TEST(result == "ORF", result);
        break;
    case OR_RAWFILE_TYPE_PEF:
        RETURN_TEST(result == "PEF", result);
        break;
    case OR_RAWFILE_TYPE_ERF:
        RETURN_TEST(result == "ERF", result);
        break;
    case OR_RAWFILE_TYPE_NRW:
        RETURN_TEST(result == "NRW", result);
        break;
    case OR_RAWFILE_TYPE_RW2:
        RETURN_TEST(result == "RW2", result);
        break;
    case OR_RAWFILE_TYPE_RAF:
        RETURN_TEST(result == "RAF", result);
        break;
    default:
        break;
    }
    RETURN_TEST(false, result);
}

bool Test::testRawTypeId(const std::string & result)
{
    RETURN_TEST_EQUALS_N(m_rawfile->typeId(),
                         boost::lexical_cast<uint32_t>(result));
}


bool Test::testThumbNum(const std::string & result)
{
    const std::vector<uint32_t> & thumbs = m_rawfile->listThumbnailSizes();
    int num = thumbs.size();
    try {
        RETURN_TEST_EQUALS_N(num, boost::lexical_cast<int>(result));
    }
    catch(...)
    {
    }
    RETURN_FAIL("conversion failed", result);
}

bool Test::testThumbSizes(const std::string & result)
{
    std::vector<uint32_t> thumbs = m_rawfile->listThumbnailSizes();
    std::vector< std::string > v;
    boost::split(v, result, boost::is_any_of(" "));
    if(v.size() != thumbs.size()) {
        RETURN_FAIL("mismatch number of elements", result);
    }
    std::vector<uint32_t> v2;
    for (const auto & s : v)
    {
        try {
            v2.push_back(boost::lexical_cast<uint32_t>(s));
        }
        catch(...)
        {
            RETURN_FAIL("conversion failed", result);
        }
    }
    RETURN_TEST(std::equal(thumbs.cbegin(), thumbs.cend(), v2.cbegin()), result);
}

bool Test::testThumbFormats(const std::string & result)
{
    bool success = true;
    auto thumbs = m_rawfile->listThumbnailSizes();
    std::vector< std::string > v;
    boost::split(v, result, boost::is_any_of(" "));
    auto result_iter = v.cbegin();
    if(v.size() != thumbs.size()) {
        RETURN_FAIL("mismatch number of elements", result);
    }
    for (const auto& thumb : thumbs) {
        Thumbnail t;
        m_rawfile->getThumbnail(thumb, t);
        success &= equalDataType(*result_iter, t.dataType());
        result_iter++;
    }
    RETURN_TEST(success, result);
}

bool Test::testThumbDataSizes(const std::string & result)
{
    bool success = true;
    auto thumbs = m_rawfile->listThumbnailSizes();
    std::vector< std::string > v;
    boost::split(v, result, boost::is_any_of(" "));
    auto result_iter = v.cbegin();
    if(v.size() != thumbs.size()) {
        RETURN_FAIL("mismatch number of elements", result);
    }
    for (const auto& thumb : thumbs) {
        Thumbnail t;
        m_rawfile->getThumbnail(thumb, t);
        try {
            success &= (boost::lexical_cast<uint32_t>(*result_iter) == t.size());
            result_iter++;
        }
        catch(...) {
            RETURN_FAIL("conversion failed", result);
        }
    }
    RETURN_TEST(success, result);
}

namespace {
uint32_t computeCrc(const Thumbnail * thumb)
{
    boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>  crc_ccitt2;

    const uint8_t * data = static_cast<uint8_t *>(thumb->data());
    size_t data_len = thumb->size();
    crc_ccitt2 = std::for_each( data, data + data_len, crc_ccitt2 );
    return crc_ccitt2();
}
}

bool Test::testThumbMd5(const std::string & result)
{
    bool success = true;
    auto thumbs = m_rawfile->listThumbnailSizes();
    std::vector< std::string > v;
    boost::split(v, result, boost::is_any_of(" "));
    auto result_iter = v.cbegin();
    if(v.size() != thumbs.size()) {
        RETURN_FAIL("mismatch number of elements", result);
    }
    for (const auto& thumb : thumbs) {
        Thumbnail t;
        m_rawfile->getThumbnail(thumb, t);
        try {
            bool succ = false;
            uint32_t crc = computeCrc(&t);
            CHECK_TEST_EQUALS(boost::lexical_cast<std::string>(crc), (*result_iter), succ);
            success &= succ;
            result_iter++;
        }
        catch(...) {
            RETURN_FAIL("conversion failed", result);
        }
    }
    RETURN_TEST(success, result);
}

namespace {
unique_ptr<RawData> loadRawData(const unique_ptr<RawFile> & file)
{
    unique_ptr<RawData> rawdata(new RawData());
    ::or_error err;
    err = file->getRawData(*rawdata, OR_OPTIONS_NONE);
    if(OR_ERROR_NONE != err) {
        rawdata.reset();
    }
    return rawdata;
}

uint32_t computeCrc(const unique_ptr<RawData> & rawdata)
{
    boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>  crc_ccitt2;

    auto data = static_cast<const uint8_t *>(rawdata->data());
    size_t data_len = rawdata->size();
    crc_ccitt2 = std::for_each( data, data + data_len, crc_ccitt2 );
    return crc_ccitt2();
}

}

bool Test::testRawDataType(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    RETURN_TEST(equalDataType(result, m_rawdata->dataType()), result);
}


bool Test::testRawDataSize(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    try {
        RETURN_TEST_EQUALS_N(m_rawdata->size(), boost::lexical_cast<uint32_t>(result));
    }
    catch(...) {
    }
    RETURN_FAIL("conversion failed", result);
}

bool Test::testRawDataDimensions(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    std::vector< std::string > v;
    boost::split(v, result, boost::is_any_of(" "));
    if(v.size() != 2) {
        RETURN_FAIL("mismatch number of elements from expected result", result);
    }
    uint32_t x, y;
    bool success = true;
    try {
        x = boost::lexical_cast<uint32_t>(v[0]);
        y = boost::lexical_cast<uint32_t>(v[1]);
        bool succ = false;
        CHECK_TEST_EQUALS_N(m_rawdata->width(), x, succ);
        success |= succ;
        CHECK_TEST_EQUALS_N(m_rawdata->height(), y,  succ);
        success |= succ;
    }
    catch(...)
    {
        RETURN_FAIL("conversion failed", result);
    }
    RETURN_TEST(success, result)
}

bool Test::testRawDataRoi(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    std::vector< std::string > v;
    boost::split(v, result, boost::is_any_of(" "));
    if(v.size() != 4) {
        RETURN_FAIL("mismatch number of elements from expected result", result);
    }
    uint32_t x, y, w, h;
    try {
        x = boost::lexical_cast<uint32_t>(v[0]);
        y = boost::lexical_cast<uint32_t>(v[1]);
        w = boost::lexical_cast<uint32_t>(v[2]);
        h = boost::lexical_cast<uint32_t>(v[3]);
    }
    catch(...)
    {
        RETURN_FAIL("conversion failed", result);
    }
    RETURN_TEST(x == m_rawdata->roi_x() && y == m_rawdata->roi_y()
        && w == m_rawdata->roi_width() && h == m_rawdata->roi_height(),
        result)
}

bool Test::testRawCfaPattern(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    bool succ = false;
    CHECK_TEST_EQUALS(cfaPatternToString(
                          m_rawdata->cfaPattern()->patternType()),
                      result, succ);
    return succ;
}

bool Test::testRawMinValue(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    uint16_t expected;
    try {
        expected = boost::lexical_cast<uint16_t>(result);
    }
    catch(...)
    {
        RETURN_FAIL("conversion failed", result);
    }
    RETURN_TEST_EQUALS_N(m_rawdata->blackLevel(), expected);
}


bool Test::testRawMaxValue(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    uint16_t expected;
    try {
        expected = boost::lexical_cast<uint16_t>(result);
    }
    catch(...)
    {
        RETURN_FAIL("conversion failed", result);
    }
    RETURN_TEST_EQUALS_N(m_rawdata->whiteLevel(), expected);
}

bool Test::testRawMd5(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }

    uint32_t crc = computeCrc(m_rawdata);

    uint32_t expected = 0;
    try {
        expected = boost::lexical_cast<uint32_t>(result);
    }
    catch(...)
    {
        RETURN_FAIL("conversion failed", result);
    }
    RETURN_TEST_EQUALS_N(crc, expected);
}


bool Test::testRawDecompressedMd5(const std::string & result)
{
    if(m_rawdata == NULL) {
        m_rawdata = loadRawData(m_rawfile);
        if(m_rawdata == NULL) {
            RETURN_FAIL("failed to get rawData", result);
        }
    }
    return false;
}


bool Test::testMetaOrientation(const std::string & result)
{
    int32_t orientation = m_rawfile->getOrientation();
    RETURN_TEST_EQUALS_N(orientation, boost::lexical_cast<int32_t>(result));
}


bool Test::testExifString(int32_t meta_index, const std::string & result)
{
    auto val = m_rawfile->getMetaValue(meta_index);
    if (val) {
        //
        auto stringVal = val->getString(0);
        RETURN_TEST_EQUALS(stringVal, result);
    }
    RETURN_FAIL("meta data not found", result);
}

bool Test::testMakerNoteCount(const std::string & result)
{
    try {
        OpenRaw::Internals::IfdFile & ifd_file =
            dynamic_cast<OpenRaw::Internals::IfdFile &>(*m_rawfile.get());
        auto exif = ifd_file.exifIfd();
        auto maker_note = exif->getMakerNoteIfd();
        if (!maker_note) {
            RETURN_FAIL("no maker not found", result);
        }
        RETURN_TEST_EQUALS_N(maker_note->numTags(),
                             boost::lexical_cast<int32_t>(result));
    }
    catch(const std::bad_cast & e) {
        RETURN_FAIL("not an IFD file", result);
    }
    catch(...) {
        RETURN_FAIL("unknown exception", result);
    }
}

bool Test::testMakerNoteId(const std::string & result)
{
    try {
        OpenRaw::Internals::IfdFile& ifd_file =
            dynamic_cast<OpenRaw::Internals::IfdFile&>(*m_rawfile.get());
        auto exif = ifd_file.exifIfd();
        auto maker_note =
            std::dynamic_pointer_cast<OpenRaw::Internals::MakerNoteDir>(
                exif->getMakerNoteIfd());
        if (!maker_note) {
            RETURN_FAIL("no maker not found", result);
        }
        RETURN_TEST_EQUALS(maker_note->getId(), result);
    }
    catch(const std::bad_cast & e) {
        RETURN_FAIL("not an IFD file", result);
    }
    catch(...) {
        RETURN_FAIL("unknown exception", result);
    }
}


/** run the test.
 * @return the number of failures. 0 means success
 */
int Test::run()
{
    // load rawfile
    fprintf(stderr, "running test %s on file %s\n", m_name.c_str(),
            m_file.c_str());

    struct stat buf;
    if(stat(m_file.c_str(), &buf) == -1) {
        fprintf(stderr, "File not found, skipping. (%d)\n", errno);
        return 0;
    }
    m_rawfile.reset(RawFile::newRawFile(m_file.c_str()));

    if(m_rawfile == NULL) {
        RETURN_FAIL("m_rawfile == NULL", std::string("not NULL"));
    }

    for(const auto & elem : m_results) {
        bool pass = false;
        switch(elem.first)
        {
        case XML_rawType:
            pass = testRawType(elem.second);
            break;
        case XML_rawTypeId:
            pass = testRawTypeId(elem.second);
            break;
        case XML_thumbNum:
            pass = testThumbNum(elem.second);
            break;
        case XML_thumbSizes:
            pass = testThumbSizes(elem.second);
            break;
        case XML_thumbFormats:
            pass = testThumbFormats(elem.second);
            break;
        case XML_thumbDataSizes:
            pass = testThumbDataSizes(elem.second);
            break;
        case XML_thumbMd5:
            pass = testThumbMd5(elem.second);
            break;
        case XML_rawDataType:
            pass = testRawDataType(elem.second);
            break;
        case XML_rawDataSize:
            pass = testRawDataSize(elem.second);
            break;
        case XML_rawDataDimensions:
            pass = testRawDataDimensions(elem.second);
            break;
        case XML_rawDataRoi:
            pass = testRawDataRoi(elem.second);
            break;
        case XML_rawCfaPattern:
            pass = testRawCfaPattern(elem.second);
            break;
        case XML_rawMinValue:
            pass = testRawMinValue(elem.second);
            break;
        case XML_rawMaxValue:
            pass = testRawMaxValue(elem.second);
            break;
        case XML_rawMd5:
            pass = testRawMd5(elem.second);
            break;
        case XML_rawDecompressedMd5:
            pass = testRawDecompressedMd5(elem.second);
            break;
        case XML_metaOrientation:
            pass = testMetaOrientation(elem.second);
            break;
        case XML_exifMake:
            pass = testExifString(META_NS_TIFF | EXIF_TAG_MAKE,
                                  elem.second);
            break;
        case XML_exifModel:
            pass = testExifString(META_NS_TIFF | EXIF_TAG_MODEL,
                                  elem.second);
            break;
        case XML_makerNoteCount:
            pass = testMakerNoteCount(elem.second);
            break;
        case XML_makerNoteId:
            pass = testMakerNoteId(elem.second);
            break;
        default:
            pass = false;
            break;
        }
        m_total++;
        if(!pass) {
            m_failure++;
        }
        else {
            m_success++;
        }
    }
    fprintf(stderr, "total %d, success %d, failure %d\n", m_total,
            m_success, m_failure);
    return m_failure;
}


void Test::merge(const Test::Ptr & t)
{
    // skip m_name
    if(!t->m_file.empty()) {
        m_file = t->m_file;
    }
    if(!t->m_source.empty()) {
        m_source = t->m_source;
    }
    // the results for t invariably replace the
    // existing on in this.
    for(const auto & elem : t->m_results) {
        m_results[elem.first] = elem.second;
    }
}


TestSuite::TestSuite()
{
}


void TestSuite::add_test(Test::Ptr && t)
{
    auto iter = m_tests.find(t->name());
    if(iter == m_tests.end()) {
        m_tests.insert(std::make_pair(t->name(), std::move(t)));
    }
    else {
        iter->second->merge(t);
    }
}


int TestSuite::load_tests(const char * testsuite_file)
{
    xml::HandlerPtr handler(new TestSuiteHandler(testsuite_file, this));

    bool has_data = false;

    has_data = handler->process();

    return !has_data;
}

int TestSuite::load_overrides(const std::string & overrides_file)
{
    xml::HandlerPtr handler(new TestSuiteHandler(overrides_file, this));

    handler->process();

    return 0;
}

namespace {

void set_file_override(xmlNode *test, const std::string & path)
{
    xmlNode * childrens = test->children;
    while(childrens) {
        if(strcmp((const char*)(childrens->name), "file") == 0) {
            xmlNodeSetContent(childrens, (const xmlChar*)path.c_str());
            return;
        }
        childrens = childrens->next;
    }
    xmlNewTextChild(test, NULL, (const xmlChar*)"file",
                    (const xmlChar*)path.c_str());
}


#if HAVE_CURL
int download(const std::string & source, CURL* handle,
                     const std::string & download_dir, std::string & dest)
{
    dest = "";
    FILE *fp = NULL;

    const char * s = source.c_str();
    const char * n = strrchr(s, '/');
    if(n) {
        n++;
        dest = download_dir + '/' + n;
    }

    if(!dest.empty()) {

        struct stat f_stat;

        if(stat(dest.c_str(), &f_stat) == -1) {
            CURLcode error;
            std::cout << "Downloading " << source
                      << " to " << dest << std::endl;

            fp = fopen(dest.c_str(), "wb");

            if(fp == NULL) {
                std::cout << " File Error " << strerror(errno) << std::endl;
                dest = "";
                return -1;
            }
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(handle, CURLOPT_URL, source.c_str());
            error = curl_easy_perform(handle);
            fclose(fp);

            if(!error) {
                std::cout << " DONE\n";
            }
            else {
                std::cout << " CURL Error " << error << std::endl;
                unlink(dest.c_str());
                dest = "";
                return -1;
            }
        }
        else {
            std::cout << dest << " exists." << std::endl;
        }
    }
    return 0;
}
#endif

}


#if HAVE_CURL
void TestSuite::walk_tests(xmlNode * testsuite, CURL* handle,
                           const std::string & download_dir)
{
    std::map<std::string, xmlNode *> overrides;
    xmlNode *test = testsuite->children;

    while(test) {
        if((test->type == XML_ELEMENT_NODE)
           && (strcmp((const char*)(test->name), "test") == 0)) {
            xmlNode * childrens = test->children;
            while(childrens) {
                if(strcmp((const char*)(childrens->name), "name") == 0) {
                    overrides.insert(std::make_pair((const char*)(childrens->name),
                                                    test));
                }
                childrens = childrens->next;
            }
        }
        test = test->next;
    }

    for(const auto & elem : m_tests) {
        std::string n = elem.first;
        std::string dest;
        int ret = download(elem.second->source(), handle, download_dir, dest);

        if(ret == 0 && !dest.empty()) {
            xmlNode * test2 = NULL;
            auto iter2 = overrides.find(n);
            if (iter2 != overrides.cend()) {
                test2 = iter2->second;
            }
            else {
                test2 = xmlNewNode(NULL, (const xmlChar*)"test");
                xmlAddChild(testsuite, test2);
                xmlNode *name_node = xmlNewTextChild(test2, NULL,
                                                     (const xmlChar*)"name",
                                                     (const xmlChar*)n.c_str());
                xmlAddChild(test2, name_node);
            }
            set_file_override(test2, dest);
        }
    }
}
#endif


namespace {

#if HAVE_CURL
int curl_write_function(void *buffer, size_t size, size_t nmemb, void *stream)
{
    FILE *fp = (FILE *) stream;

    size_t w;
    w = fwrite(buffer, size, nmemb, fp);
    if (w < size * nmemb) {

    }
    else {
        std::cout << ".";
    }

    return (int) w;
}
#endif

}


#if HAVE_CURL
int TestSuite::bootstrap(const std::string & overrides_file,
                         const std::string & download_dir)
{
    xmlDocPtr doc;
    CURL *handle;

    handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_write_function);

    doc = xmlReadFile(overrides_file.c_str(), NULL, 0);
    xmlNode *root_element;
    if (doc) {
        root_element = xmlDocGetRootElement(doc);
    }
    else {
        doc = xmlNewDoc((const xmlChar *)"1.0");
        root_element = xmlNewDocNode(doc,
                                     NULL, (xmlChar*)"testsuite", NULL);
        xmlDocSetRootElement(doc, root_element);
    }


    if(root_element->type == XML_ELEMENT_NODE) {
        if(strcmp((const char*)(root_element->name), "testsuite") == 0) {
            walk_tests(root_element, handle, download_dir);
        }
    }

    xmlSaveFormatFile(overrides_file.c_str(), doc, XML_SAVE_FORMAT);
    xmlFreeDoc(doc);
    curl_easy_cleanup(handle);
    return 0;
}
#else
int TestSuite::bootstrap(const std::string & /*overrides_file*/,
                         const std::string & /*download_dir*/)
{
    return 1;
}
#endif


int TestSuite::run_all()
{
    int failures = 0;
    for(const auto & elem : m_tests) {
        failures += elem.second->run();
    }
    return failures;
}

int main(int argc, char ** argv)
{
    bool bootstrap = false;
    std::string download_dir;
    const char * srcdir = getenv("srcdir");
    if(srcdir == NULL) {
        srcdir = "./";
    }

    int opt;
    while ((opt = getopt(argc, argv, "bd:")) != -1) {
        switch(opt) {
        case 'b':
#if HAVE_CURL
            bootstrap = true;
#else
            fprintf(stderr, "Bootstraping is disabled. Please rebuild "
                    "with CURL support. Quitting.\n");
            return 1;
#endif
            break;
        case 'd':
            if(optarg[0] != '/') {
#ifdef HAVE_GET_CURRENT_DIR_NAME
                char * dir = get_current_dir_name();
#else
                char * dir = (char *) malloc(PATH_MAX * sizeof(char));
                getcwd(dir, PATH_MAX);
#endif
                download_dir = dir;
                download_dir += '/';
                download_dir += optarg;
                free(dir);
            }
            else {
                download_dir = optarg;
            }
            break;
        default:
            break;
        }
    }

    std::string testsuite_file = srcdir;
    testsuite_file += "/";
    testsuite_file += (argv[optind] ? argv[optind] : "testsuite.xml");

    or_debug_set_level(ERROR);

    TestSuite testsuite;
    testsuite.load_tests(testsuite_file.c_str());
    std::string override_file = testsuite_file + ".overrides";
    if(!bootstrap) {
        testsuite.load_overrides(override_file);
        return testsuite.run_all();
    }
    else {
        testsuite.bootstrap(override_file, download_dir);
    }
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
