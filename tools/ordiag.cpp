/*
 * libopenraw - ordiag.cpp
 *
 * Copyright (C) 2007-2008 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>

#include <libopenraw++/rawfile.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>
using OpenRaw::RawFile;
using OpenRaw::Thumbnail;
using OpenRaw::RawData;

/**
 * Dump on RawFile. (functor)
 */
class OrDiag
{
public:
	/** constructor
	 * @param out the output stream
	 */
	OrDiag(std::ostream & out)
		: m_out(out)
		{
		}

	std::string cfaPatternToString(RawData::CfaPattern t) 
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

	std::string dataTypeToString(Thumbnail::DataType t)
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
			case OR_DATA_TYPE_CFA:
				return "CFA data";
				break;
			case OR_DATA_TYPE_COMPRESSED_CFA:
				return "Compressed CFA data";
				break;
			case OR_DATA_TYPE_UNKNOWN:
				return "Unknown type";
				break;
			default:
				break;
			}
			return "Invalid";
		}

	/** return a string for the raw file type
	 */
	std::string typeToString(RawFile::Type t)
		{
			switch(t) {
			case OR_RAWFILE_TYPE_UNKNOWN:
				break;
			case OR_RAWFILE_TYPE_CR2:
				return "Canon CR2";
				break;
			case OR_RAWFILE_TYPE_CRW:
				return "Canon CRW";
				break;
			case OR_RAWFILE_TYPE_NEF:
				return "Nikon NEF";
				break;
			case OR_RAWFILE_TYPE_MRW:
				return "Minolta MRW";
				break;
			case OR_RAWFILE_TYPE_ARW:
				return "Sony ARW";
				break;
			case OR_RAWFILE_TYPE_DNG:
				return "Adobe DNG";
				break;
			case OR_RAWFILE_TYPE_ORF:
				return "Olympus ORF";
				break;
			case OR_RAWFILE_TYPE_PEF:
				return "Pentax PEF";
				break;
			case OR_RAWFILE_TYPE_ERF:
				return "Epson ERF";
				break;
			default:
				break;
			}
			return "Unknown";
		}

	/** dump the previews of the raw file to mout
	 */
	void dumpPreviews(const boost::scoped_ptr<RawFile> & rf)
		{
			const std::vector<uint32_t> & previews = rf->listThumbnailSizes();
			m_out << boost::format("\tNumber of previews: %1%\n") 
				% previews.size();
			
			m_out << "\tAvailable previews:\n";
			for(std::vector<uint32_t>::const_iterator iter = previews.begin();
					iter != previews.end(); iter++)	{

				m_out << boost::format("\t\tSize %1%\n") % *iter;

				Thumbnail thumb;
				::or_error err = rf->getThumbnail(*iter, thumb);
				if (err != OR_ERROR_NONE) {
					m_out << boost::format("\t\t\tError getting thumbnail %1%\n") % err;
				}
				else {
					m_out << boost::format("\t\t\tFormat %1%\n") 
						% dataTypeToString(thumb.dataType());
					m_out << boost::format("\t\t\tDimensions: x = %1% y = %2%\n")
						% thumb.x() % thumb.y();
					m_out << boost::format("\t\t\tByte size: %1%\n") 
						% thumb.size();
				}
			}
		}

	void dumpRawData(const boost::scoped_ptr<RawFile> & rf)
		{
			RawData rd;
			::or_error err = rf->getRawData(rd, 0);
			if (err == OR_ERROR_NONE) {
				m_out << "\tRAW data\n";
				m_out << boost::format("\t\tType: %1%")
					% dataTypeToString(rd.dataType());
				if(rd.dataType() == OR_DATA_TYPE_COMPRESSED_CFA)  {
					m_out << boost::format(" (compression = %1%)\n") % rd.compression();
				}
				else {
					m_out << "\n";
				}
				m_out << boost::format("\t\tByte size: %1%\n")
					% rd.size();
				m_out << boost::format("\t\tDimensions: x = %1% y = %2%\n")
					% rd.x() % rd.y();
				m_out << boost::format("\t\tBayer Type: %1%\n")
					% cfaPatternToString(rd.cfaPattern());
			}
			else {
				m_out << boost::format("\tNo Raw Data found! (error = %1%)\n")
					% err;
			}
		}
	void dumpMetaData(const boost::scoped_ptr<RawFile> & rf)
		{
			int32_t o;
			o = rf->getOrientation();
			m_out << "\tMeta data\n";
			m_out << boost::format("\t\tOrientation: %1%\n")
				% o;
		}
	void operator()(const std::string &s)
		{
			m_out << boost::format("Dumping %1%\n") % s;

			boost::scoped_ptr<RawFile> rf(RawFile::newRawFile(s.c_str()));

			if (rf == NULL) {
				m_out << "unrecognized file\n";
			}
			else {
				m_out << boost::format("\tType = %1% (%2%)\n") % rf->type() 
															 % typeToString(rf->type());
				dumpPreviews(rf);
				dumpRawData(rf);
				dumpMetaData(rf);
			}
		}
private:
	std::ostream & m_out;
};


void print_help()
{
	std::cerr << "ordiag [-v] [-h] [-d 0-9] [files...]\n";
	std::cerr << "Print libopenraw diagnostics\n";
	std::cerr << "\t-h: show this help\n";
	std::cerr << "\t-v: show version\n";
	std::cerr << "\t-d level: set debug / verbosity to level\n";
	std::cerr << "\tfiles: the files to diagnose\n";
}

void print_version()
{
	std::cerr << "ordiag version 0.0 - (c) 2007 Hubert Figuiere\n";
}



int main(int argc, char **argv)
{
	int done = 0;
	int dbl = 0;
	std::vector<std::string> files;

	OpenRaw::init();

	int o;
	while((o = getopt(argc, argv, "hvd")) != -1) {
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
			dbl++;
			break;
		case '?':
			break;
		default:
			break;
		}
	}
	if (done) {
		return 0;
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
	for_each(files.begin(), files.end(), OrDiag(std::cout));

	return 0;
}
