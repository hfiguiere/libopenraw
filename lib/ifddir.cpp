/*
 * libopenraw - ifddir.cpp
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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


#include <libopenraw/types.h>

#include "debug.h"
#include "exception.h"
#include "ifd.h"
#include "io/stream.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		bool IFDDir::isPrimary::operator()(const Ref &dir)
		{
			uint32_t subtype = 1; 
			return dir->getValue(IFD::EXIF_TAG_NEW_SUBFILE_TYPE, subtype)
				&& (subtype == 0);
		}

		bool IFDDir::isThumbnail::operator()(const Ref &dir)
		{
			uint32_t subtype = 0; 
			return dir->getValue(IFD::EXIF_TAG_NEW_SUBFILE_TYPE, subtype)
				&& (subtype == 1);
		}

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
			IO::Stream *file = m_container.file();
			m_entries.clear();
			file->seek(m_offset, SEEK_SET);
			m_container.readInt16(file, numEntries);

			for(int16_t i = 0; i < numEntries; i++) {
				uint16_t id;
				int16_t type;
				int32_t count;
				uint32_t data;
				m_container.readUInt16(file, id);
				m_container.readInt16(file, type);
				m_container.readInt32(file, count);
				file->read(&data, 4);
				IFDEntry::Ref entry(new IFDEntry(id, type, 
												 count, data, m_container));
				m_entries[id] = entry;
			}

			return true;
		}

		IFDEntry::Ref IFDDir::getEntry(uint16_t id) const
		{
			std::map<uint16_t, IFDEntry::Ref>::const_iterator iter;
			iter = m_entries.find(id);
			if (iter != m_entries.end()) {
				return iter->second;
			}
			return IFDEntry::Ref((IFDEntry*)NULL);
		}


		bool IFDDir::getIntegerValue(uint16_t id, uint32_t &v)
		{
			bool success = false;
			IFDEntry::Ref e = getEntry(id);
			if (e != NULL) {
				try {
					switch(e->type())
					{
					case IFD::EXIF_FORMAT_LONG:
						v = IFDTypeTrait<uint32_t>::get(*e);
						success = true;
						break;
					case IFD::EXIF_FORMAT_SHORT:
						v = IFDTypeTrait<uint16_t>::get(*e);
						success = true;
						break;
					default:
						break;
					}
				}
				catch(const std::exception & ex) {
					Trace(ERROR) << "Exception raised " << ex.what() 
											 << " fetch integer value for " << id << "\n";
				}
			}
			return success;
		}


		off_t IFDDir::nextIFD()
		{
			int16_t numEntries;
			IO::Stream *file = m_container.file();

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
		IFDDir::Ref IFDDir::getSubIFD(uint32_t idx) const
		{
			std::vector<uint32_t> offsets;
			IFDEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);
			if (e != NULL) {
				try {
					e->getArray(offsets);
					if (idx >= offsets.size()) {
						Ref ref(new IFDDir(offsets[idx], m_container));
						ref->load();
						return ref;
					}
				}
				catch(const std::exception &ex) {
					Trace(ERROR) << "Exception " << ex.what() << "\n";
				}
			}
			return Ref(static_cast<IFDDir*>(NULL));
		}


		bool IFDDir::getSubIFDs(std::vector<IFDDir::Ref> & ifds) 
		{
			bool success = false;
			std::vector<uint32_t> offsets;
			IFDEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);
			if (e != NULL) {
				try {
					e->getArray(offsets);
					for (std::vector<uint32_t>::const_iterator iter = offsets.begin();
							 iter != offsets.end(); iter++) {
						Ref ifd(new IFDDir(*iter, m_container));
						ifd->load();
						ifds.push_back(ifd);
					}
					success = true;
				}
				catch(const std::exception &ex) {
					Trace(ERROR) << "Exception " << ex.what() << "\n";					
				}
			}
			return success;
		}

		/** The SubIFD is locate at offset found in the field
		 * EXIF_TAG_SUB_IFDS
		 */
		IFDDir::Ref IFDDir::getExifIFD()
		{
			bool success = false;
			uint32_t val_offset = 0;
			success = getValue(IFD::EXIF_TAG_EXIF_IFD_POINTER, val_offset);
			if (success) {
				Trace(DEBUG1) << "Exif IFD offset (uncorrected) = " << val_offset 
							 << "\n";
				val_offset += m_container.exifOffsetCorrection();
				Trace(DEBUG1) << "Exif IFD offset = " << val_offset << "\n";
				Ref ref(new IFDDir(val_offset, m_container));
				ref->load();
				return ref;
			}
			else {
				Trace(DEBUG1) << "Exif IFD offset not found.\n";				
			}
			return Ref(static_cast<IFDDir*>(NULL));
		}

	}
}

