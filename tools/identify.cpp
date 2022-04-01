/*
 * libopenraw - identify.cpp
 *
 * Copyright (C) 2022 Hubert Figuière
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


#include <iostream>
#include <string>
#include <vector>

#include <libopenraw/libopenraw.h>

void print_help()
{
    std::cerr << "identify [-h] [-d 0-9] [files...]\n";
    std::cerr << "Print libopenraw diagnostics\n";
    std::cerr << "\t-h: show this help\n";
    std::cerr << "\t-d level: set debug / verbosity to level\n";
    std::cerr << "\tfiles: the files to diagnose\n";
}

void print_version()
{
    std::cerr << "identify version 0.1 - (c) 2022 Hubert Figuiere\n";
}

int main(int argc, char **argv)
{
    int done = 0;
    int dbl = 0;
    std::string extract_thumbs;
    std::vector<std::string> files;

    int o;
    while((o = getopt(argc, argv, "hvdDt:")) != -1) {
        switch (o) {
        case 'h':
            print_help();
            done = 1;
            break;
        case 'd':
            dbl++;
            break;
        case '?':
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
        if (dbl) {
            print_version();
        }
        print_help();
        return 1;
    }

    if (dbl >=2) {
        or_debug_set_level(DEBUG2);
    }
    // do the business.
    for_each(files.begin(), files.end(), [dbl] (const std::string& file) {
        if (dbl) {
            printf("Processing %s\n", file.c_str());
        }
        ORRawFileRef rf = or_rawfile_new(file.c_str(), OR_RAWFILE_TYPE_UNKNOWN);
        if (rf != nullptr) {
            auto id = or_rawfile_get_typeid(rf);
            printf("%s %u\n", file.c_str(), id);
            or_rawfile_release(rf);
        } else {
            printf("Unrecognized: %s\n", file.c_str());
        }
    });

    return 0;
}
/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  tab-width:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
