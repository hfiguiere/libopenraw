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
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <libopenraw/libopenraw.h>

#include "dumputils.hpp"

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
    default:
        return "INVALID";
    }
}

class ExifDump
{
public:
    ExifDump(std::ostream& out)
        : m_out(out)
        {
        }

    void operator()(const std::string &s)
        {
            m_out << boost::format("EXIF from '%1%'\n") % s;

            ORRawFileRef rf = or_rawfile_new(s.c_str(), OR_RAWFILE_TYPE_UNKNOWN);

            if (rf == nullptr) {
                m_out << "unrecognized file\n";
            } else {
                dump_file_info(m_out, rf);

                ORMetadataIteratorRef iter = or_rawfile_get_metadata_iterator(rf);
                m_out << "EXIF metadata\n";

                while (or_metadata_iterator_next(iter)) {
                    uint16_t id;
                    ExifTagType type;
                    if (or_metadata_iterator_get_entry(iter, &id, &type)) {
                        m_out << boost::format("\t0x%1$x = %2%\n") % id % map_exif_type(type);
                    }
                }

                or_metadata_iterator_free(iter);
            }

            or_rawfile_release(rf);
        }
private:
    std::ostream & m_out;
};

void print_help()
{
    std::cerr << "exifdump [-v] [-h] [-d 0-9] [files...]\n";
    std::cerr << "Dump EXIF from raw file\n";
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
    std::vector<std::string> files;

    int o;
    while ((o = getopt(argc, argv, "hvd")) != -1) {
        switch (o) {
        case 'h':
            print_help();
            done = 1;
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
    for_each(files.begin(), files.end(), ExifDump(std::cout));

}
