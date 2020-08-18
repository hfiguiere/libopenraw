/*
 * libopenraw - exifdump.cpp
 *
 * Copyright (C) 2020 Hubert Figuière
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

#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <libopenraw/libopenraw.h>

#include "dumputils.hpp"
#include "exif/exif_tags.hpp"

const char* map_ifd_type(or_ifd_dir_type type)
{
    switch(type) {
    case OR_IFD_OTHER:
        return "OTHER Metadata";
    case OR_IFD_MAIN:
        return "Image Metadata";
    case OR_IFD_EXIF:
        return "Exif Metadata";
    case OR_IFD_MNOTE:
        return "MakerNote Metadata";
    case OR_IFD_RAW:
        return "RAW Metadata";
    case OR_IFD_SUBIFD:
        return "Sub IFD";
    }
    return "INVALID";
}

const char* map_exif_type(ExifTagType type)
{
    switch(type) {
    case EXIF_FORMAT_BYTE:
        return "BYTE";
    case EXIF_FORMAT_ASCII:
        return "ASCII";
    case EXIF_FORMAT_SHORT:
        return "SHORT";
    case EXIF_FORMAT_LONG:
        return "LONG";
    case EXIF_FORMAT_RATIONAL:
        return "RATIONAL";
    case EXIF_FORMAT_SBYTE:
        return "SBYTE";
    case EXIF_FORMAT_UNDEFINED:
        return "UNDEFINED";
    case EXIF_FORMAT_SSHORT:
        return "SSHORT";
    case EXIF_FORMAT_SLONG:
        return "SLONG";
    case EXIF_FORMAT_SRATIONAL:
        return "SRATIONAL";
    case EXIF_FORMAT_FLOAT:
        return "FLOAT";
    case EXIF_FORMAT_DOUBLE:
        return "DOUBLE";
    }
    return "INVALID";
}

class ExifDump
{
public:
    ExifDump(std::ostream& out, bool dump_binaries)
        : m_out(out)
        , m_dump_binaries(dump_binaries)
        {
        }

    void operator()(const std::string &s)
        {
            m_out << boost::format("EXIF from '%1%'\n") % s;

            ORRawFileRef rf = or_rawfile_new(s.c_str(), OR_RAWFILE_TYPE_UNKNOWN);

            if (rf == nullptr) {
                m_out << "unrecognized file\n";
            } else {
                dump_file_info(m_out, rf, false);

                ORMetadataIteratorRef iter = or_rawfile_get_metadata_iterator(rf);
                or_ifd_dir_type last_ifd_type = OR_IFD_OTHER;

                while (or_metadata_iterator_next(iter)) {
                    ORIfdDirRef ifd = nullptr;
                    uint16_t id;
                    ExifTagType type;
                    ORMetaValueRef value = nullptr;
                    if (or_metadata_iterator_get_entry(iter, &ifd, &id, &type, &value)) {
                        or_ifd_dir_type ifd_type = or_ifd_get_type(ifd);
                        if (ifd_type != last_ifd_type) {
                            m_out << boost::format("%1% - %2% entries\n") %
                                map_ifd_type(ifd_type) % or_ifd_count_tags(ifd);
                            last_ifd_type = ifd_type;
                            const char* makernote_id = or_ifd_get_makernote_id(ifd);
                            if (makernote_id) {
                                m_out << boost::format("MakerNote type %1%\n") % makernote_id;
                            }
                        }
                        const char* tagname = or_ifd_get_tag_name(ifd, id);
                        uint32_t count = or_metavalue_get_count(value);
                        m_out << boost::format("\t0x%1$x %2% = %3% [ %4% ]\n") % id %
                            (tagname ? std::string(tagname) : "") %
                            map_exif_type(type) % count;
                        if (value) {
                            switch (type) {
                            case EXIF_FORMAT_ASCII:
                                m_out << boost::format("\tvalue = %1%\n") %
                                    or_metavalue_get_string(value, 0);
                                break;
                            default:
                                if (type != EXIF_FORMAT_UNDEFINED || m_dump_binaries) {
                                    m_out << boost::format("\tvalue = %1%\n") %
                                        or_metavalue_get_as_string(value, m_dump_binaries);
                                } else {
                                    m_out << "\tvalue output skipped, use -b to dump\n";
                                }
                            }
                            or_metavalue_release(value);
                        } else {
                            m_out << "\tNo value\n";
                        }
                        or_ifd_release(ifd);
                    }
                }

                or_metadata_iterator_free(iter);
            }

            or_rawfile_release(rf);
        }
private:
    std::ostream & m_out;
    bool m_dump_binaries;
};

void print_help()
{
    std::cerr << "exifdump [-v] [-h] [-d 0-9] [files...]\n";
    std::cerr << "Dump EXIF from raw file\n";
    std::cerr << "\t-b: dump binaries\n";
    std::cerr << "\t-h: show this help\n";
    std::cerr << "\t-v: show version\n";
    std::cerr << "\t-d level: set debug / verbosity to level\n";
    std::cerr << "\tfiles: the files to diagnose\n";
}

void print_version()
{
    std::cerr << "exifdump version 0.2.0 - (c) 2020 Hubert Figuière\n";
}

int main(int argc, char **argv)
{
    int done = 0;
    int dbg = 0;
    bool dump_binaries = false;
    std::vector<std::string> files;

    int o;
    while ((o = getopt(argc, argv, "hvdb")) != -1) {
        switch (o) {
        case 'h':
            print_help();
            done = 1;
            break;
        case 'b':
            dump_binaries = true;
            break;
        case 'v':
            print_version();
            done = 1;
            break;
        case 'd':
            dbg++;
            break;
        default:
            break;
        }
    }

    if (done) {
        return 1;
    }

    for ( ; optind < argc; optind++) {
        files.push_back(argv[optind]);
    }

    if (files.empty()) {
        std::cerr << "missing file name.\n";
        if (dbg) {
            print_version();
        }
        print_help();
        return 1;
    }


    if (dbg >=2) {
        or_debug_set_level(DEBUG2);
    }

    // do the business.
    for_each(files.begin(), files.end(), ExifDump(std::cout, dump_binaries));
}
