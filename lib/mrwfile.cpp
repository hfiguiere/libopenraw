/*
 * libopenraw - mrwfile.cpp
 *
 * Copyright (C) 2006,2008 Hubert Figuiere
 * Copyright (C) 2008 Bradley Broom
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
#include <boost/scoped_array.hpp>
#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
#include "io/stream.h"
#include "io/file.h"
#include "mrwcontainer.h"
#include "ifd.h"
#include "mrwfile.h"
#include "unpack.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		const struct IFDFile::camera_ids_t MRWFile::s_def[] = {
			{ "21860002", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_MAXXUM_5D) },
			{ "21810002", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_MAXXUM_7D) },
			{ "27730001", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_DIMAGE5) },
			{ "27660001", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_DIMAGE7) },
			{ "27790001", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_DIMAGE7I) },
			{ "27780001", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_DIMAGE7HI) },
			{ "27820001", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_A1) },
			{ "27200001", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_A2) },
			{ "27470002", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA,
											  OR_TYPEID_MINOLTA_A200) },
			{ 0, 0 }
		};

		RawFile *MRWFile::factory(IO::Stream *_f)
		{
			return new MRWFile(_f);
		}

		MRWFile::MRWFile(IO::Stream* _f)
			: IFDFile(_f, OR_RAWFILE_TYPE_MRW, false)
		{
			_setIdMap(s_def);
			m_container = new MRWContainer (m_io, 0);
		}

		MRWFile::~MRWFile()
		{
		}

		IFDDir::Ref  MRWFile::_locateCfaIfd()
		{
			// in MRW the CFA IFD is the main IFD
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}
			return m_mainIfd;
		}


		IFDDir::Ref  MRWFile::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}


		void MRWFile::_identifyId()
		{
			MRWContainer *mc = (MRWContainer *)m_container;
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}

			if(mc->prd) {
				std::string version = mc->prd->string_val(MRW::PRD_VERSION);
				_setTypeId(_typeIdFromModel(version));
			}
		}

	
		/* This code only knows about Dimage 5/7, in which the thumbnail position is special. */
		::or_error MRWFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
		{
			::or_error err = OR_ERROR_NOT_FOUND;
			list.push_back (640);
			err = OR_ERROR_NONE;
			return err;
		}

		/* This code only knows about Dimage 5/7, in which the thumbnail position is special. */
		::or_error MRWFile::_getThumbnail(uint32_t /*size*/, Thumbnail & thumbnail)
		{
			IFDDir::Ref dir;
			IFDEntry::Ref maker_ent;	/* Make note directory entry. */
			IFDEntry::Ref thumb_ent;	/* Thumbnail data directory entry. */
			::or_error ret = OR_ERROR_NOT_FOUND;
			MRWContainer *mc = (MRWContainer *)m_container;
			
			dir = _locateExifIfd();
			if (!dir) {
				Trace(WARNING) << "EXIF dir not found\n";
				return ret;
			}

			maker_ent = dir->getEntry(IFD::EXIF_TAG_MAKER_NOTE);
			if (!maker_ent) {
				Trace(WARNING) << "maker note offset entry not found\n";
				return ret;
			}
			uint32_t off = 0;
			off = maker_ent->offset();

			IFDDir::Ref ref(new IFDDir(mc->ttw->offset() + 
									   MRW::DataBlockHeaderLength + off, 
									   *m_container));
			ref->load();
			
			uint32_t tnail_offset = 0;
			uint32_t tnail_len = 0;
			thumb_ent = ref->getEntry(MRW::MRWTAG_THUMBNAIL);
			if (thumb_ent) {
				tnail_offset = thumb_ent->offset();
				tnail_len = thumb_ent->count();
			}
			else if(ref->getValue(MRW::MRWTAG_THUMBNAIL_OFFSET, tnail_offset)) {
				if(!ref->getValue(MRW::MRWTAG_THUMBNAIL_LENGTH, tnail_len)) {
					Trace(WARNING) << "thumbnail lenght entry not found\n";
					return ret;
				}
			}
			else 
			{
				Trace(WARNING) << "thumbnail offset entry not found\n";
				return ret;
			}
			
			Trace(DEBUG1) << "thumbnail offset found, "
						  << " offset == " << tnail_offset  << " count == " 
						  << tnail_len << "\n";
			void *p = thumbnail.allocData (tnail_len);
			size_t fetched = m_container->fetchData(p, mc->ttw->offset() 
													+ MRW::DataBlockHeaderLength 
													+ tnail_offset, 
													tnail_len);
			if (fetched != tnail_len) {
				Trace(WARNING) << "Unable to fetch all thumbnail data: " 
							   << fetched << " not " << tnail_len 
							   << " bytes\n";
			}
			/* Need to patch first byte. */
			((unsigned char *)p)[0] = 0xFF;

			thumbnail.setDataType (OR_DATA_TYPE_JPEG);
			thumbnail.setDimensions (640, 480);
			return OR_ERROR_NONE;
		}


		::or_error MRWFile::_getRawData(RawData & data, uint32_t options) 
		{ 
			MRWContainer *mc = (MRWContainer *)m_container;

			if(!mc->prd) {
				return OR_ERROR_NOT_FOUND;
			}
			/* Obtain sensor dimensions from PRD block. */
			uint16_t y = mc->prd->uint16_val (MRW::PRD_SENSOR_LENGTH);
			uint16_t x = mc->prd->uint16_val (MRW::PRD_SENSOR_WIDTH);
			uint8_t bpc =  mc->prd->uint8_val (MRW::PRD_PIXEL_SIZE);

			bool is_compressed = (mc->prd->uint8_val(MRW::PRD_STORAGE_TYPE) == 0x59);
			/* Allocate space for and retrieve pixel data.
			 * Currently only for cameras that don't compress pixel data.
			 */
			/* Set pixel array parameters. */
			uint32_t finaldatalen = 2 * x * y;
			uint32_t datalen =
				(is_compressed ? x * y + ((x * y) >> 1) : finaldatalen);

			if(options & OR_OPTIONS_DONT_DECOMPRESS) {
				finaldatalen = datalen;
			}
			if(is_compressed && (options & OR_OPTIONS_DONT_DECOMPRESS)) {
				data.setDataType (OR_DATA_TYPE_COMPRESSED_CFA);
			}
			else {
				data.setDataType (OR_DATA_TYPE_CFA);
			}
			data.setBpc(bpc);
			// this seems to be the hardcoded value.
			data.setMax(0xf7d);
			Trace(DEBUG1) << "datalen = " << datalen <<
				" final datalen = " << finaldatalen << "\n";
			void *p = data.allocData(finaldatalen);
			size_t fetched = 0;
			off_t offset = mc->pixelDataOffset();
			if(!is_compressed || (options & OR_OPTIONS_DONT_DECOMPRESS)) {
				fetched = m_container->fetchData (p, offset, datalen);
			}
			else {
				Unpack unpack(x, IFD::COMPRESS_NONE);
				size_t blocksize = unpack.block_size();
				boost::scoped_array<uint8_t> block(new uint8_t[blocksize]);
				uint8_t * outdata = (uint8_t*)data.data();
				size_t got;
				do {
					Trace(DEBUG2) << "fatchData @offset " << offset << "\n";
					got = m_container->fetchData (block.get(), 
												  offset, blocksize);
					fetched += got;
					offset += got;
					Trace(DEBUG2) << "got " << got << "\n";
					if(got) {
						size_t out = unpack.unpack_be12to16(outdata,
															block.get(), got);
						outdata += out;
						Trace(DEBUG2) << "unpacked " << out
									  << " bytes from " << got << "\n";
					}
				} while((got != 0) && (fetched < datalen));
			}
			if (fetched < datalen) {
				Trace(WARNING) << "Fetched only " << fetched <<
					" of " << datalen << ": continuing anyway.\n";
			}
			uint16_t bpat = mc->prd->uint16_val (MRW::PRD_BAYER_PATTERN);
			or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NONE;
			switch(bpat) 
			{
			case 0x0001:
				cfa_pattern = OR_CFA_PATTERN_RGGB;
				break;
			case 0x0004:
				cfa_pattern = OR_CFA_PATTERN_GBRG;
				break;
			default:
				break;
			}
			data.setCfaPattern(cfa_pattern);
			data.setDimensions (x, y);

			return OR_ERROR_NONE; 
		}

	}
}
