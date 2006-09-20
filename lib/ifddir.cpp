/*
 * libopenraw - ifddir.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <libopenraw/types.h>

#include "debug.h"
#include "ifd.h"
#include "iofile.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		IFDDir::IFDDir(off_t _offset, IFDFileContainer & _container)
			: m_offset(_offset), m_container(_container), 
				m_entries()
		{
			
		}

		IFDDir::~IFDDir()
		{

		}

		bool IFDDir::load()
		{
			Trace(DEBUG1) << "IFDDir::load() m_offset =" << m_offset << "\n";
			int16_t numEntries = 0;
			IOFile *file = m_container.file();
			m_entries.clear();
			file->seek(m_offset, SEEK_SET);
			m_container.readInt16(file, numEntries);

			Trace(DEBUG1) << "num =" << numEntries << "\n";

			for(int16_t i = 0; i < numEntries; i++) {
				int16_t id;
				int16_t type;
				int32_t count;
				int32_t offset;
				m_container.readInt16(file, id);
				m_container.readInt16(file, type);
				m_container.readInt32(file, count);
				m_container.readInt32(file, offset);
				IFDEntry::Ref entry(new IFDEntry(id, type, 
																				 count, offset, m_container));
				Trace(DEBUG1) << "adding elem for " << id << "\n";
				m_entries[id] = entry;
			}

			return true;
		}

		IFDEntry::Ref IFDDir::getEntry(int id)
		{
			return m_entries[id];
		}


		bool IFDDir::getLongValue(int id, long &v)
		{
			bool success = false;
			IFDEntry::Ref e = getEntry(id);
			if (e != NULL) {
				v = e->getLong();
				success = true;
			}
			return success;
		}


		bool IFDDir::getShortValue(int id, short &v)
		{
			bool success = false;
			IFDEntry::Ref e = getEntry(id);
			if (e != NULL) {
				v = e->getShort();
				success = true;
			}
			return success;
		}

		off_t IFDDir::nextIFD()
		{
			int16_t numEntries;
			IOFile *file = m_container.file();

			if(m_entries.size() == 0) {
				file->seek(m_offset, SEEK_SET);
				m_container.readInt16(file, numEntries);
				Trace(DEBUG1) << "numEntries =" << numEntries 
									<< " shifting " << (numEntries * 12) + 2
									<< "bytes\n";
			}
			else {
				numEntries = m_entries.size();
			}

			file->seek(m_offset + (numEntries * 12) + 2, SEEK_SET);
			int32_t next;
			m_container.readInt32(file, next);
			return next;
		}
		
		/** The SubIFD is locate at offset found in the field
		 * EXIF_TAG_SUB_IFDS
		 */
		IFDDir::Ref IFDDir::getSubIFD()
		{
			bool success;
			long offset = 0;
			success = getLongValue(IFD::EXIF_TAG_SUB_IFDS, offset);
			if (success) {
				Ref ref(new IFDDir(offset, m_container));
			}
			return Ref(static_cast<IFDDir*>(NULL));
		}

		/** The SubIFD is locate at offset found in the field
		 * EXIF_TAG_SUB_IFDS
		 */
		IFDDir::Ref IFDDir::getExifIFD()
		{
			bool success;
			long offset = 0;
			success = getLongValue(IFD::EXIF_TAG_EXIF_IFD_POINTER, offset);
			if (success) {
				Ref ref(new IFDDir(offset, m_container));
			}
			return Ref(static_cast<IFDDir*>(NULL));
		}

	}
}

