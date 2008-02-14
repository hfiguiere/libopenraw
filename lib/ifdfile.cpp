/*
 * libopenraw - ifdfile.cpp
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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

#include <algorithm>
#include <numeric>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "debug.h"
#include "io/stream.h"
#include "io/streamclone.h"
#include "io/file.h"
#include "ifd.h"
#include "ifdfile.h"
#include "ifdfilecontainer.h"
#include "jfifcontainer.h"
#include "metavalue.h"
#include "unpack.h"

using namespace Debug;
using boost::scoped_ptr;


namespace OpenRaw {
	namespace Internals {


		IFDFile::IFDFile(const char *_filename, Type _type, 
						 bool instantiateContainer)
			: RawFile(_filename, _type),
				m_thumbLocations(),
				m_io(new IO::File(_filename)),
				m_container(NULL)
		{
			if(instantiateContainer) {
				m_container = new IFDFileContainer(m_io, 0);
			}
		}

		IFDFile::~IFDFile()
		{
			delete m_container;
			delete m_io;
		}

		// this one seems to be pretty much the same for all the
		// IFD based raw files
		IFDDir::Ref  IFDFile::_locateExifIfd()
		{
			m_mainIfd = _locateMainIfd();
			if (!m_mainIfd) {
				Trace(ERROR) << "IFDFile::_locateExifIfd() "
					"main IFD not found\n";
				return IFDDir::Ref();
			}
			return m_mainIfd->getExifIFD();
		}


		::or_error IFDFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
		{
			::or_error err = OR_ERROR_NONE;

			Trace(DEBUG1) << "_enumThumbnailSizes()\n";
			std::vector<IFDDir::Ref> & dirs = m_container->directories();
			std::vector<IFDDir::Ref>::iterator iter; 
			
			Trace(DEBUG1) << "num of dirs " << dirs.size() << "\n";
			int c = 0;
			for(iter = dirs.begin(); iter != dirs.end(); ++iter, ++c)
			{
				IFDDir::Ref & dir = *iter;
				dir->load();
				or_error ret = _locateThumbnail(dir, list);
				if (ret == OR_ERROR_NONE)
				{
					Trace(DEBUG1) << "Found " << list.back() << " pixels\n";
				}
			}
			if (list.size() <= 0) {
				err = OR_ERROR_NOT_FOUND;
			}
			return err;
		}


		::or_error IFDFile::_locateThumbnail(const IFDDir::Ref & dir,
																	 std::vector<uint32_t> &list)
		{
			::or_error ret = OR_ERROR_NOT_FOUND;
			bool got_it;
			uint32_t x = 0;
			uint32_t y = 0;
			::or_data_type _type = OR_DATA_TYPE_NONE;
			uint32_t subtype = 0;

			Trace(DEBUG1) << "_locateThumbnail\n";

			got_it = dir->getValue(IFD::EXIF_TAG_NEW_SUBFILE_TYPE, subtype);
			Trace(DEBUG1) << "subtype " << subtype  << "\n";
			if (!got_it || (subtype == 1)) {

				uint16_t photom_int = 0;
				got_it = dir->getValue(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION, 
																		photom_int);

				if (got_it) {
					Trace(DEBUG1) << "photometric int " << photom_int  << "\n";
				}
				// photometric interpretation is RGB
				if (!got_it || (photom_int == 2)) {

					got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
					got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);

					uint32_t offset = 0;
					got_it = dir->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
					if (!got_it) {
						got_it = dir->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT,
																			 offset);
 						Trace(DEBUG1) << "looking for JPEG at " << offset << "\n";
						if (got_it) {
							_type = OR_DATA_TYPE_JPEG;
							if (x == 0 || y == 0) {
								scoped_ptr<IO::StreamClone> s(new IO::StreamClone(m_io, offset));
								scoped_ptr<JFIFContainer> jfif(new JFIFContainer(s.get(), 0));
								if (jfif->getDimensions(x,y)) {
									Trace(DEBUG1) << "JPEG dimensions x=" << x 
																<< " y=" << y << "\n";
								}
								else {
									_type = OR_DATA_TYPE_NONE;
								}
							}
						}
					}
					else {
						Trace(DEBUG1) << "found strip offsets\n";
						if (x != 0 && y != 0) {
							_type = OR_DATA_TYPE_PIXMAP_8RGB;
						}
					}
					if(_type != OR_DATA_TYPE_NONE) {
						uint32_t dim = std::max(x, y);
						m_thumbLocations[dim] = IFDThumbDesc(x, y, _type, dir);
						list.push_back(dim);
						ret = OR_ERROR_NONE;
					}
				}
			}

			return ret;
		}


		::or_error IFDFile::_getThumbnail(uint32_t size, Thumbnail & thumbnail)
		{
			::or_error ret = OR_ERROR_NOT_FOUND;
			ThumbLocations::iterator iter = m_thumbLocations.find(size);
			if(iter != m_thumbLocations.end()) 
			{
				bool got_it;

				IFDThumbDesc & desc = iter->second;
				thumbnail.setDataType(desc.type);
				uint32_t byte_length= 0; /**< of the buffer */
				uint32_t offset = 0;
				uint32_t x = desc.x;
				uint32_t y = desc.y;

				switch(desc.type)
				{
				case OR_DATA_TYPE_JPEG:
					got_it = desc.ifddir
						->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH,
													 byte_length);
					got_it = desc.ifddir
						->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT,
													 offset);
					break;
				case OR_DATA_TYPE_PIXMAP_8RGB:
					got_it = desc.ifddir
						->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
					got_it = desc.ifddir
						->getValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_length);

					got_it = desc.ifddir
						->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
					got_it = desc.ifddir
						->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);
					break;
				default:
					break;
				}
				if (byte_length != 0) {
					void *p = thumbnail.allocData(byte_length);
					size_t real_size = m_container->fetchData(p, offset, 
																										byte_length);
					if (real_size < byte_length) {
						Trace(WARNING) << "Size mismatch for data: ignoring.\n";
					}

					thumbnail.setDimensions(x, y);
					ret = OR_ERROR_NONE;
				}
			}

			return ret;
		}


		MetaValue *IFDFile::_getMetaValue(int32_t meta_index)
		{
			MetaValue * val = NULL;
			IFDDir::Ref ifd;
			if(META_INDEX_MASKOUT(meta_index) == META_NS_TIFF) {
				if(!m_mainIfd) {
					m_mainIfd = _locateMainIfd();
				}
				ifd = m_mainIfd;
			}
			else if(META_INDEX_MASKOUT(meta_index) == META_NS_EXIF) {
				if(!m_exifIfd) {
					m_exifIfd = _locateExifIfd();
				}
				ifd = m_exifIfd;
			}
			else {
				Trace(ERROR) << "Unknown Meta Namespace\n";
			}
			if(ifd) {
				Trace(DEBUG1) << "Meta value for " 
							  << META_NS_MASKOUT(meta_index) << "\n";
				uint16_t n = 0;
				bool got_it = ifd->getValue(META_NS_MASKOUT(meta_index), n);
				if(got_it){
					Trace(DEBUG1) << "found value\n";
					val = new MetaValue(boost::any(static_cast<int32_t>(n)));
				}				
			}
			return val;
		}


		namespace {
		/** convert the CFA Pattern as stored in the entry */
		RawData::CfaPattern _convertCfaPattern(const IFDEntry::Ref & e)
		{
			std::vector<uint8_t> cfaPattern;
			RawData::CfaPattern cfa_pattern = OR_CFA_PATTERN_NONE;
			
			e->getArray(cfaPattern);
			if(cfaPattern.size() != 4) {
				Trace(WARNING) << "Unsupported bayer pattern\n";
			}
			else {
				Trace(DEBUG2) << "patter is = " << cfaPattern[0] << ", "
							  << cfaPattern[1] << ", " << cfaPattern[2] 
							  << ", " << cfaPattern[3] << "\n";
				switch(cfaPattern[0]) {
				case IFD::CFA_RED:
					switch(cfaPattern[1]) {
					case IFD::CFA_GREEN:
						if((cfaPattern[2] == IFD::CFA_GREEN) && (cfaPattern[3] == IFD::CFA_BLUE)) {
							cfa_pattern = OR_CFA_PATTERN_RGGB;
						}
						break;
					}
					break;
				case IFD::CFA_GREEN:
					switch(cfaPattern[1]) {
					case IFD::CFA_RED:
						if((cfaPattern[2] == 2) && (cfaPattern[3] == IFD::CFA_GREEN)) {
							cfa_pattern = OR_CFA_PATTERN_GRBG;
						}
						break;
					case 2:
						if((cfaPattern[2] == IFD::CFA_RED) && (cfaPattern[3] == IFD::CFA_GREEN)) {
							cfa_pattern = OR_CFA_PATTERN_GBRG;
						}
						break;
					}
					break;
				case IFD::CFA_BLUE:
					switch(cfaPattern[1]) {
					case IFD::CFA_GREEN:
						if((cfaPattern[2] == IFD::CFA_GREEN) && (cfaPattern[3] == IFD::CFA_RED)) {
							cfa_pattern = OR_CFA_PATTERN_BGGR;
						}
						break;
					}
					break;
				}
				//
			}
			return cfa_pattern;
		}

		/** get the CFA Pattern out of the directory
		 * @param dir the directory
		 * @return the cfa_pattern value. %OR_CFA_PATTERN_NONE mean that
		 * nothing has been found.
		 */
		static RawData::CfaPattern _getCfaPattern(const IFDDir::Ref & dir)
		{
			RawData::CfaPattern cfa_pattern = OR_CFA_PATTERN_NONE;
			try {
				IFDEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_CFA_PATTERN);
				if(e) {
					cfa_pattern = _convertCfaPattern(e);
				}
			}
			catch(...)
			{
				Trace(ERROR) << "Exception in _getCfaPattern().\n";
			}
			return cfa_pattern;
		}

		} // end anon namespace

		::or_error IFDFile::_getRawDataFromDir(RawData & data, IFDDir::Ref & dir)
		{
			::or_error ret = OR_ERROR_NONE;
			
			uint16_t bpc = 0;
			uint32_t offset = 0;
			uint32_t byte_length = 0;
			bool got_it;
			uint32_t x, y;
			x = 0;
			y = 0;

			got_it = dir->getValue(IFD::EXIF_TAG_BITS_PER_SAMPLE, bpc);
			if(!got_it) {
				Trace(ERROR) << "unable to guess Bits per sample\n";
			}

			got_it = dir->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
			if(got_it) {
				IFDEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
				if(e) {
					std::vector<uint32_t> counts;
					e->getArray(counts);
					Trace(DEBUG1) << "counting tiles\n";
					byte_length = std::accumulate(counts.begin(), counts.end(), 0);
				}
				else {
					Trace(DEBUG1) << "byte len not found\n";
					return OR_ERROR_NOT_FOUND;
				}
			}
			else {
				// the tile are individual JPEGS....
				// TODO extract all of them.
				IFDEntry::Ref e = dir->getEntry(IFD::TIFF_TAG_TILE_OFFSETS);
				if(e) {
					std::vector<uint32_t> offsets;
					e->getArray(offsets);
					if(offsets.size() > 1) {
						offset = offsets[0];
					}
					else {
						Trace(DEBUG1) << "tile offsets empty\n";
						return OR_ERROR_NOT_FOUND;						
					}
				}
				else {
					Trace(DEBUG1) << "tile offsets not found\n";
					return OR_ERROR_NOT_FOUND;						
				}
				e = dir->getEntry(IFD::TIFF_TAG_TILE_BYTECOUNTS);
				if(e) {
					std::vector<uint32_t> counts;
					e->getArray(counts);
					Trace(DEBUG1) << "counting tiles\n";
					byte_length = std::accumulate(counts.begin(), counts.end(), 0);
				}
				else {
					Trace(DEBUG1) << "tile byte counts not found\n";
					return OR_ERROR_NOT_FOUND;						
				}
			}
			got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
			if(!got_it) {
				Trace(DEBUG1) << "X not found\n";
				return OR_ERROR_NOT_FOUND;
			}
			got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);
			if(!got_it) {
				Trace(DEBUG1) << "Y not found\n";
				return OR_ERROR_NOT_FOUND;
			}

			uint32_t compression = 0;
			got_it = dir->getIntegerValue(IFD::EXIF_TAG_COMPRESSION, compression);
			if(!got_it)
			{
				Trace(DEBUG1) << "Compression type not found\n";
			}
			BitmapData::DataType data_type = OR_DATA_TYPE_NONE;

			switch(compression) 
			{
			case IFD::COMPRESS_NONE:
			case IFD::COMPRESS_NIKON:
				data_type = OR_DATA_TYPE_CFA;
				break;
			default:
				data_type = OR_DATA_TYPE_COMPRESSED_CFA;
				break;
			}
			
			RawData::CfaPattern cfa_pattern = _getCfaPattern(dir);

			if((bpc == 16) || (data_type == OR_DATA_TYPE_COMPRESSED_CFA)) {
				void *p = data.allocData(byte_length);
				size_t real_size = m_container->fetchData(p, offset, 
														  byte_length);
				if (real_size < byte_length) {
					Trace(WARNING) << "Size mismatch for data: ignoring.\n";
				}
			}
			else if(bpc == 12) {
				size_t fetched = 0;
				const off_t blocksize = 128*1024 + 1; // must be a multiple of 3
				boost::scoped_array<uint8_t> block(new uint8_t[blocksize]);
				size_t outleft = x * y * 2;
				uint8_t * outdata = (uint8_t*)data.allocData(outleft);
				size_t got;
				do {
					Trace(DEBUG2) << "fatchData @offset " << offset << "\n";
					got = m_container->fetchData (block.get(), 
												  offset, blocksize);
					fetched += got;
					offset += got;
					Trace(DEBUG2) << "got " << got << "\n";
					if(got) {
						size_t out = unpack_be12to16(outdata, outleft, 
													 block.get(), got);
						outdata += out;
						outleft -= out;
						Trace(DEBUG2) << "unpacked " << out
									  << " bytes from " << got << "\n";
					}
				} while((got != 0) && (fetched < byte_length));
			}
			else {
				Trace(ERROR) << "Unsupported bpc " << bpc << "\n";
			}
			data.setCfaPattern(cfa_pattern);
			data.setDataType(data_type);
			data.setDimensions(x, y);
			
			return ret;
		}

	}
}
