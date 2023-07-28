/*
 * libopenraw - ordiag.cpp
 *
 * Copyright (C) 2007-2020 Hubert Figuière
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


#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <libopenraw/libopenraw.h>

#include "dumputils.hpp"

/** @addtogroup tools
 * @{
 */

/**
 * @brief Dump a RawFile. (functor)
 */
class OrDiag
{
public:
    /** @brief Constructor
     * @param out The output stream
     * @param extract_thumbs If "all" extract all thumbnails, otherwise try
     *   to guess the size.
     * @param dev_mode If true the output format for devlopment.
     */
    OrDiag(std::ostream & out, const std::string & extract_thumbs, bool dev_mode)
        : m_out(out)
        , m_extract_all_thumbs(false)
        , m_dev_mode(dev_mode)
        {
            m_extract_all_thumbs = (extract_thumbs == "all");
            if (!m_extract_all_thumbs) {
                try {
                    int size = boost::lexical_cast<int>(extract_thumbs);
                    m_thumb_sizes.insert(size);
                }
                catch(...) {

                }
            }
        }

    std::string cfaPatternToString(ORMosaicInfoRef pattern)
        {
            if(pattern == NULL) {
                return "(null)";
            }

            std::string out;
            uint16_t size = 0;
            const uint8_t* patternPattern
                = or_mosaicinfo_get_pattern(pattern, &size);

            for(uint16_t i = 0; i < size; ++i) {
                switch(patternPattern[i]) {
                case OR_PATTERN_COLOUR_RED:
                    out.push_back('R');
                    break;
                case OR_PATTERN_COLOUR_GREEN:
                    out.push_back('G');
                    break;

                case OR_PATTERN_COLOUR_BLUE:
                    out.push_back('B');
                    break;

                default:
                    out.push_back('*');
                    break;
                }
            }

            return out;
        }
    std::string cfaPatternToString(::or_cfa_pattern t)
        {
            switch(t) {
            case OR_CFA_PATTERN_NONE:
                return "None";
                break;
            case OR_CFA_PATTERN_NON_RGB22:
                return "Non RGB 2x2";
                break;
            case OR_CFA_PATTERN_RGGB:
                return "R,G,G,B";
                break;
            case OR_CFA_PATTERN_GBRG:
                return "G,B,R,G";
                break;
            case OR_CFA_PATTERN_BGGR:
                return "B,G,G,R";
                break;
            case OR_CFA_PATTERN_GRBG:
                return "G,R,B,G";
                break;
            default:
                break;
            }
            return str(boost::format("Unknown %1%") % t);
        };

    std::string dataTypeToString(or_data_type t)
        {
            switch(t) {
            case OR_DATA_TYPE_NONE:
                return "None";
                break;
            case OR_DATA_TYPE_PIXMAP_8RGB:
                return "8bits per channel RGB pixmap";
                break;
            case OR_DATA_TYPE_JPEG:
                return "JPEG data";
                break;
            case OR_DATA_TYPE_TIFF:
                return "TIFF container";
                break;
            case OR_DATA_TYPE_PNG:
                return "PNG container";
                break;
            case OR_DATA_TYPE_RAW:
                return "RAW data";
                break;
            case OR_DATA_TYPE_COMPRESSED_RAW:
                return "Compressed RAW data";
                break;
            case OR_DATA_TYPE_UNKNOWN:
                return "Unknown type";
                break;
            default:
                break;
            }
            return "Invalid";
        }

    /** @brief Extract thumbnail to a file
     * @return The filename. If empty, nothing was done.
     */
    std::string extractThumb(ORThumbnailRef thumb)
        {
            FILE* f;
            size_t s;
            std::string ext;
            switch(or_thumbnail_format(thumb)) {
            case OR_DATA_TYPE_PIXMAP_8RGB:
                ext = "ppm";
                break;
            case OR_DATA_TYPE_JPEG:
                ext = "jpg";
                break;
            default:
                break;
            }
            if (ext.empty()) {
                return "";
            }

            uint32_t x, y;
            or_thumbnail_dimensions(thumb, &x, &y);
            uint32_t dim = std::max(x, y);
            std::string name(str(boost::format("thumb_%1%.%2%") % dim  % ext));
            f = fopen(name.c_str(), "wb");
            if (or_thumbnail_format(thumb) == OR_DATA_TYPE_PIXMAP_8RGB) {
                // ppm preemble.
                fprintf(f, "P6\n");
                fprintf(f, "%d %d\n", x, y);
                fprintf(f, "%d\n", /*(componentsize == 2) ? 0xffff :*/ 0xff);
            }
            size_t dataSize = or_thumbnail_data_size(thumb);
            s = fwrite(or_thumbnail_data(thumb), 1, dataSize, f);
            if(s != dataSize) {
                std::cerr << "short write of " << s << " bytes\n";
            }
            fclose(f);

            return name;
        }

    /** @brief Dump the previews of the raw file to the output stream.
     */
    void dumpPreviews(ORRawFileRef rf)
        {
            size_t size = 0;
            const uint32_t * previews = or_rawfile_get_thumbnail_sizes(rf, &size);
            m_out << boost::format("\tNumber of previews: %1%\n")
                % size;

            m_out << "\tAvailable previews:\n";
            for(size_t i = 0; i < size; i++) {

                m_out << boost::format("\t\tSize %1%\n") % previews[i];

                ::or_error err = OR_ERROR_NONE;
                ORThumbnailRef thumb = or_rawfile_get_thumbnail(rf, previews[i], &err);
                if (err != OR_ERROR_NONE) {
                    m_out << boost::format("\t\t\tError getting thumbnail %1%\n") % err;
                }
                else {
                    m_out << boost::format("\t\t\tFormat %1%\n")
                        % dataTypeToString(or_thumbnail_format(thumb));
                    uint32_t x, y;
                    or_thumbnail_dimensions(thumb, &x, &y);
                    m_out << boost::format("\t\t\tDimensions: width = %1% height = %2%\n")
                        % x % y;
                    m_out << boost::format("\t\t\tByte size: %1%\n")
                        % or_thumbnail_data_size(thumb);
                }
                if (m_extract_all_thumbs
                    || m_thumb_sizes.find(previews[i]) != m_thumb_sizes.end()) {

                    std::string name = extractThumb(thumb);

                    m_out << boost::format("\t\t\tOutput as %1%\n") % name;
                }
                or_thumbnail_release(thumb);
            }
        }

    void dumpRawData(ORRawFileRef rf)
        {
            ::or_error err = OR_ERROR_NONE;
            ORRawDataRef rd = or_rawfile_get_rawdata(rf, 0, &err);
            if (err == OR_ERROR_NONE) {
                m_out << "\tRAW data\n";
                or_data_type dataType = or_rawdata_format(rd);
                m_out << boost::format("\t\tType: %1%")
                    % dataTypeToString(dataType);
                if(dataType == OR_DATA_TYPE_COMPRESSED_RAW)  {
                    m_out << boost::format(" (compression = %1%)\n")
                        % or_rawdata_get_compression(rd);
                }
                else {
                    m_out << "\n";
                }
                m_out << boost::format("\t\tByte size: %1%\n")
                    % or_rawdata_data_size(rd);
                uint32_t x, y;
                or_rawdata_dimensions(rd, &x, &y);
                m_out << boost::format("\t\tDimensions: width = %1% height = %2%\n")
                    % x % y;

                // Active Area
                uint32_t aa_x, aa_y, aa_width, aa_height;
                or_rawdata_get_active_area(rd, &aa_x, &aa_y,
                                           &aa_width, &aa_height);
                m_out <<
                    boost::format("\t\tActive Area (x,y,w,h): %1% %2% %3% %4%\n")
                    % aa_x % aa_y % aa_width % aa_height;

                // CFA
                ORMosaicInfoRef pattern = or_rawdata_get_mosaicinfo(rd);
                ::or_cfa_pattern patternType
                      = pattern ? or_mosaicinfo_get_type(pattern)
                      : OR_CFA_PATTERN_NON_RGB22;
                m_out << boost::format("\t\tBayer Type: %1%\n")
                    % cfaPatternToString(patternType);

                if(patternType == OR_CFA_PATTERN_NON_RGB22) {
                    m_out << boost::format("\t\tPattern: %1%\n")
                        % cfaPatternToString(pattern);
                }

                m_out << boost::format("\t\tBits per channel: %1%\n")
                    % or_rawdata_bpc(rd);
                uint16_t black, white;
                or_rawdata_get_levels(rd, &black, &white);
                m_out << boost::format(
                    "\t\tValues: black = %1% white = %2%\n") % black % white;

                uint32_t matrix_size = 0;
                const double *matrix = or_rawdata_get_colour_matrix(rd, 1, &matrix_size);
                if (matrix) {
                    m_out << boost::format("\t\tColour Matrix 1: ");
                    for (uint32_t i = 0; i < matrix_size; i++) {
                        if (i > 0) {
                            m_out << ", ";
                        }
                        m_out << matrix[i];
                    }
                    m_out << "\n";
                }
            }
            else {
                m_out << boost::format("\tNo Raw Data found! (error = %1%)\n")
                    % err;
            }
            or_rawdata_release(rd);
        }
    void dumpMetaData(ORRawFileRef rf)
        {
            int32_t o = or_rawfile_get_orientation(rf);
            m_out << "\tMeta data\n";
            m_out << "\t\tMakerNotes\n";

            auto mnote = or_rawfile_get_ifd(rf, OR_IFD_MNOTE);
            if (!mnote) {
                m_out << "\t\t\tNo MakerNote found!\n";
            } else {
                const char* makernote_id = or_ifd_get_makernote_id(mnote);
                m_out << boost::format("\t\t\tType = %1%\n")
                    % (makernote_id ? makernote_id : "(null)");
                auto num_entries = or_ifd_count_tags(mnote);
                m_out << boost::format("\t\t\tNum entries = %1%\n")
                    % num_entries;
                mnote = nullptr;
            }
            m_out << boost::format("\t\tOrientation: %1%\n")
                % o;
            double matrix[9];
            uint32_t size = 9;


            auto origin = or_rawfile_get_colour_matrix_origin(rf);
            std::string os;
            switch (origin) {
            case OR_COLOUR_MATRIX_BUILTIN:
                os = "Built-in";
                break;
            case OR_COLOUR_MATRIX_PROVIDED:
                os = "Provided";
                break;
            default:
                os = "Unknown";
            }
            m_out << boost::format("\t\tColour Matrix Origin: %1%\n")
                % os;

            ExifLightsourceValue calIll = static_cast<ExifLightsourceValue>(or_rawfile_get_calibration_illuminant1(rf));
            m_out << boost::format("\t\tCalibration Illuminant 1: %1%\n")
                % static_cast<int>(calIll);

            ::or_error err = or_rawfile_get_colourmatrix1(rf, matrix, &size);
            if(err == OR_ERROR_NONE) {
                if (m_dev_mode) {
                    std::vector<int64_t> int_matrix;
                    std::transform(&matrix[0], &matrix[9], std::back_inserter(int_matrix),
                                   [](double v) -> int64_t { return rintl(v * 10000.0); });
                    m_out << boost::format("\t\tColour Matrix 1: %1%, %2%, %3%, "
                                           "%4%, %5%, %6%, %7%, %8%, %9%\n")
                        % int_matrix[0] % int_matrix[1] % int_matrix[2]
                        % int_matrix[3] % int_matrix[4] % int_matrix[5]
                        % int_matrix[6] % int_matrix[7] % int_matrix[8];
                } else {
                    m_out << boost::format("\t\tColour Matrix 1: %1%, %2%, %3%, "
                                           "%4%, %5%, %6%, %7%, %8%, %9%\n")
                        % matrix[0] % matrix[1] % matrix[2]
                        % matrix[3] % matrix[4] % matrix[5]
                        % matrix[6] % matrix[7] % matrix[8];
                }
            }
            else {
                m_out << "\t\tNo Colour Matrix 1\n";
            }

            calIll = static_cast<ExifLightsourceValue>(or_rawfile_get_calibration_illuminant2(rf));
            m_out << boost::format("\t\tCalibration Illuminant 2: %1%\n")
                % static_cast<int>(calIll);

            size = 9;
            err = or_rawfile_get_colourmatrix2(rf, matrix, &size);
            if(err == OR_ERROR_NONE) {
                if (m_dev_mode) {
                    std::vector<int64_t> int_matrix;
                    std::transform(&matrix[0], &matrix[9], std::back_inserter(int_matrix),
                                   [](double v) -> int64_t { return rintl(v * 10000.0); });
                    m_out << boost::format("\t\tColour Matrix 2: %1%, %2%, %3%, "
                                           "%4%, %5%, %6%, %7%, %8%, %9%\n")
                        % int_matrix[0] % int_matrix[1] % int_matrix[2]
                        % int_matrix[3] % int_matrix[4] % int_matrix[5]
                        % int_matrix[6] % int_matrix[7] % int_matrix[8];

                } else {
                    m_out << boost::format("\t\tColour Matrix 2: %1%, %2%, %3%, "
                                           "%4%, %5%, %6%, %7%, %8%, %9%\n")
                        % matrix[0] % matrix[1] % matrix[2]
                        % matrix[3] % matrix[4] % matrix[5]
                        % matrix[6] % matrix[7] % matrix[8];
                }
            }
            else {
                m_out << "\t\tNo Colour Matrix 2\n";
            }
        }
    void operator()(const std::string &s)
        {
            m_out << boost::format("Dumping %1%\n") % s;

            ORRawFileRef rf = or_rawfile_new(s.c_str(), OR_RAWFILE_TYPE_UNKNOWN);

            //std::unique_ptr<RawFile> rf(RawFile::newRawFile(s.c_str()));

            if (rf == NULL) {
                m_out << "unrecognized file\n";
            }
            else {
                dump_file_info(m_out, rf, m_dev_mode);

                dumpPreviews(rf);
                dumpRawData(rf);
                dumpMetaData(rf);
            }
            or_rawfile_release(rf);
        }
private:
    std::ostream & m_out;
    bool m_extract_all_thumbs;
    bool m_dev_mode;
    std::set<int> m_thumb_sizes;
};


void print_help()
{
    std::cerr << "ordiag [-v] [-h] [-t all|<size>] [-d 0-9] [files...]\n";
    std::cerr << "Print libopenraw diagnostics\n";
    std::cerr << "\t-h: show this help\n";
    std::cerr << "\t-D: developer mode: display some data a format suited for development\n";
    std::cerr << "\t-v: show version\n";
    std::cerr << "\t-d level: set debug / verbosity to level\n";
    std::cerr << "\t-t [all|<size>]: extract thumbnails. all or <size>.\n";
    std::cerr << "\tfiles: the files to diagnose\n";
}

void print_version()
{
    std::cerr << "ordiag version 0.1 - (c) 2007-2014 Hubert Figuiere\n";
}



int main(int argc, char **argv)
{
    int done = 0;
    int dbl = 0;
    bool dev_mode = false;
    std::string extract_thumbs;
    std::vector<std::string> files;

    int o;
    while((o = getopt(argc, argv, "hvdDt:")) != -1) {
        switch (o) {
        case 'h':
            print_help();
            done = 1;
            break;
        case 'v':
            print_version();
            done = 1;
            break;
        case 'D':
            dev_mode = true;
            break;
        case 'd':
            dbl++;
            break;
        case 't':
            if(optarg) {
                extract_thumbs = optarg;
            }
            else {
                print_help();
                done = 1;
            }
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
    for_each(files.begin(), files.end(), OrDiag(std::cout, extract_thumbs, dev_mode));

    return 0;
}

/** @} */
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
