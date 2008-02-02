/*
 * libopenraw - mrwfile.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
 * Copyright (C) 2008 Bradley Broom
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


#include <iostream>
#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "debug.h"
#include "io/stream.h"
#include "io/file.h"
#include "mrwcontainer.h"
#include "ifd.h"
#include "mrwfile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *MRWFile::factory(const char* _filename)
		{
			return new MRWFile(_filename);
		}

		MRWFile::MRWFile(const char* _filename)
			: IFDFile(_filename, OR_RAWFILE_TYPE_MRW, false)
		{
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
			
			dir = MRWFile::_locateExifIfd();
			if (!dir) {
				Trace(WARNING) << "EXIF dir not found\n";
				return ret;
			}

			uint32_t off;
			if (!dir->getValue(IFD::EXIF_TAG_MAKER_NOTE, off)) {
				Trace(WARNING) << "maker note offset entry not found\n";
				return ret;
			}

			IFDDir::Ref ref(new IFDDir(mc->ttw->offset() + MRW::DataBlockHeaderLength + off, *m_container));
			ref->load();
			
			thumb_ent = ref->getEntry(MRW::MRWTAG_THUMBNAIL);
			if (thumb_ent)
				Trace(DEBUG1) << "thumbnail offset found, type == " << thumb_ent->type() 
							  << " offset == " << thumb_ent->offset() << " count == " 
							  << thumb_ent->count() << "\n";
			else {
				Trace(WARNING) << "thumbnail offset entry not found\n";
				return ret;
			}

			size_t length = thumb_ent->count();
			void *p = thumbnail.allocData (length);
			size_t fetched = m_container->fetchData (p, mc->ttw->offset() + MRW::DataBlockHeaderLength 
													 + thumb_ent->offset(), length);
			if (fetched != length) {
				Trace(WARNING) << "Unable to fetch all thumbnail data: " << fetched << " not " << length << " bytes\n";
			}
			/* Need to patch first byte. */
			((unsigned char *)p)[0] = 0xFF;

			thumbnail.setDataType (OR_DATA_TYPE_JPEG);
			thumbnail.setDimensions (640, 480);
			return OR_ERROR_NONE;
		}

		::or_error MRWFile::_getRawData(RawData & data, uint32_t /*options*/) 
		{ 
			MRWContainer *mc = (MRWContainer *)m_container;

			/* Obtain sensor dimensions from PRD block. */
			uint16_t y = mc->prd->uint16_val (MRW::PRD_SENSOR_LENGTH);
			uint16_t x = mc->prd->uint16_val (MRW::PRD_SENSOR_WIDTH);

			/* Allocate space for and retrieve pixel data.
			 * Currently only for cameras that don't compress pixel data.
			 */
			uint32_t datalen = 2 * x * y;
			void *p = data.allocData (datalen);
			size_t fetched = m_container->fetchData (p, mc->pixelDataOffset(), datalen);
			if (fetched < datalen) {
				Trace(WARNING) << "Unable to fetch all required data: continuing anyway.\n";
			}

			/* Set pixel array parameters. */
			data.setDataType (OR_DATA_TYPE_CFA);
			data.setDimensions (x, y);

			return OR_ERROR_NONE; 
		}

	}
}
