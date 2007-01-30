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
#include "exception.h"
#include "ifd.h"
#include "io/stream.h"
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
			IO::Stream *file = m_container.file();
			m_entries.clear();
			file->seek(m_offset, SEEK_SET);
			m_container.readInt16(file, numEntries);

			Trace(DEBUG1) << "num =" << numEntries << "\n";

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
				Trace(DEBUG1) << "adding elem for " << id << "\n";
				m_entries[id] = entry;
			}

			return true;
		}

		IFDEntry::Ref IFDDir::getEntry(int id)
		{
			return m_entries[id];
		}


		bool IFDDir::getIntegerValue(int id, uint32_t &v)
		{
			bool success = false;
			IFDEntry::Ref e = getEntry(id);
			if (e != NULL) {
				try {
					switch(e->type())
					{
					case IFD::EXIF_FORMAT_LONG:
						v = e->getLong();
						success = true;
						break;
					case IFD::EXIF_FORMAT_SHORT:
						v = e->getShort();
						success = true;
						break;
					default:
						break;
					}
				}
				catch(const std::exception & e) {
					Trace(ERROR) << "Exception raised " << e.what() 
											 << " fetch integer value for " << id << "\n";
				}
			}
			return success;
		}


		bool IFDDir::getLongValue(int id, uint32_t &v)
		{
			bool success = false;
			IFDEntry::Ref e = getEntry(id);
			if (e != NULL) {
				try {
					v = e->getLong();
					success = true;
				}
				catch(const std::exception & e) {
					Trace(ERROR) << "Exception raised " << e.what() 
											 << " fetch long value for " << id << "\n";
				}
			}
			return success;
		}


		bool IFDDir::getShortValue(int id, uint16_t &v)
		{
			bool success = false;
			IFDEntry::Ref e = getEntry(id);
			if (e != NULL) {
				try {
					v = e->getShort();
					success = true;
				}
				catch(const std::exception & e) {
					Trace(ERROR) << "Exception raised " << e.what() 
											 << " fetch long value for " << id << "\n";
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
		IFDDir::Ref IFDDir::getSubIFD()
		{
			bool success;
			uint32_t offset = 0;
			success = getLongValue(IFD::EXIF_TAG_SUB_IFDS, offset);
			if (success) {
				Ref ref(new IFDDir(offset, m_container));
				ref->load();
				return ref;
			}
			return Ref(static_cast<IFDDir*>(NULL));
		}

		/** The SubIFD is locate at offset found in the field
		 * EXIF_TAG_SUB_IFDS
		 */
		IFDDir::Ref IFDDir::getExifIFD()
		{
			bool success = false;
			uint32_t offset = 0;
			success = getLongValue(IFD::EXIF_TAG_EXIF_IFD_POINTER, offset);
			if (success) {
				Trace(DEBUG1) << "Exif IFD offset = " << offset << "\n";
				Ref ref(new IFDDir(offset, m_container));
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

