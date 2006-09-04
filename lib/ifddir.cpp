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

#include <iostream>

#include <libopenraw/types.h>

#include "iofile.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"

namespace OpenRaw {
	namespace Internals {

		IFDDir::IFDDir(off_t _offset, IFDFileContainer & _container)
			: m_offset(_offset), m_container(_container)
		{
			
		}

		IFDDir::~IFDDir()
		{

		}

		bool IFDDir::load()
		{
			std::cerr << "IFDDir::load() m_offset =" << m_offset << std::endl;
			Int16 numEntries = 0;
			IOFile *file = m_container.file();
			m_entries.clear();
			file->seek(m_offset, SEEK_SET);
			m_container.readInt16(file, numEntries);

			std::cerr << "num =" << numEntries << std::endl;

			for(Int16 i = 0; i < numEntries; i++) {
				Int16 id;
				Int16 type;
				Int32 count;
				Int32 offset;
				m_container.readInt16(file, id);
				m_container.readInt16(file, type);
				m_container.readInt32(file, count);
				m_container.readInt32(file, offset);
				IFDEntry::Ref entry(new IFDEntry(id, type, 
																				 count, offset, m_container));
				std::cerr << "adding elem for " << id << std::endl;
				m_entries[id] = entry;
			}

			return true;
		}

		IFDEntry::Ref IFDDir::getEntry(int id)
		{
//			std::cerr << "num entries = " << m_entries.size() << std::endl;
			return m_entries[id];
		}


		off_t IFDDir::nextIFD()
		{
			Int16 numEntries;
			IOFile *file = m_container.file();

			if(m_entries.size() == 0) {
				file->seek(m_offset, SEEK_SET);
				m_container.readInt16(file, numEntries);
				std::cerr << "numEntries =" << numEntries 
									<< " shifting " << (numEntries * 12) + 2
									<< "bytes" << std::endl;
			}
			else {
				numEntries = m_entries.size();
			}

			file->seek(m_offset + (numEntries * 12) + 2, SEEK_SET);
			Int32 next;
			m_container.readInt32(file, next);
			return next;
		}
		
	}
}

