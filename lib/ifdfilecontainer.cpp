/*
 * libopenraw - ifdfilecontainer.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <sys/types.h>

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>

#include "debug.h"

#include "ifdfilecontainer.h"
#include "io/file.h"


using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		IFDFileContainer::IFDFileContainer(IO::Stream *_file, off_t offset)
			: RawContainer(_file, offset), 
			  m_error(0),
			  m_exif_offset_correction(0),
			  m_current_dir(),
			  m_dirs()
		{
		}
	
		IFDFileContainer::~IFDFileContainer()
		{
			m_dirs.clear();
		}


		IFDFileContainer::EndianType 
		IFDFileContainer::isMagicHeader(const char *p, int len)
		{
			if (len < 4){
				// we need at least 4 bytes to check
				return ENDIAN_NULL;
			}
			if ((p[0] == 0x49) && (p[1] == 0x49)
					&& (p[2] == 0x2a) && (p[3] == 0x00)) {
				return ENDIAN_LITTLE;
			}
			else if ((p[0] == 0x4d) && (p[1] == 0x4d)
							 && (p[2] == 0x00) && (p[3] == 0x2a)) {
				return ENDIAN_BIG;
			}
			return ENDIAN_NULL;
		}


		int IFDFileContainer::countDirectories(void)
		{
			if (m_dirs.size() == 0) {
				// FIXME check result
				bool ret = _locateDirs();
				if (!ret) {
					return -1;
				}
			}
			return m_dirs.size();
		}

		std::vector<IFDDir::Ref> & 
		IFDFileContainer::directories()
		{
			if (m_dirs.size() == 0) {
				countDirectories();
			}
			return m_dirs;
		}

		IFDDir::Ref
		IFDFileContainer::setDirectory(int dir)
		{
			if (dir < 0) {
				// FIXME set error
				return IFDDir::Ref((IFDDir*)NULL);
			}
			// FIXME handle negative values
			int n = countDirectories();
			if (n <= 0) {
				// FIXME set error
				return IFDDir::Ref((IFDDir*)NULL);
			}
			// dir is signed here because we can pass negative 
			// value for specific Exif IFDs.
			if (dir > (int)m_dirs.size()) {
				// FIXME set error
				return IFDDir::Ref((IFDDir*)NULL);
			}
			m_current_dir = m_dirs[dir];
			m_current_dir->load();
			return m_current_dir;
		}


		size_t 
		IFDFileContainer::getDirectoryDataSize()
		{
			// TODO move to IFDirectory
			Trace(DEBUG1) << "getDirectoryDataSize()" << "\n";
			off_t offset = m_current_dir->offset();
			// FIXME check error
			Trace(DEBUG1) << "offset = " << offset 
								<< " m_numTags = " << m_current_dir->numTags() << "\n";
			off_t begin = offset + 2 + (m_current_dir->numTags()*12);
			
			Trace(DEBUG1) << "begin = " << begin << "\n";

			m_file->seek(begin, SEEK_SET);
			begin += 2;
			int32_t nextIFD;
			readInt32(m_file, nextIFD);
			Trace(DEBUG1) << "nextIFD = " << nextIFD << "\n";
			if (nextIFD == 0) {
				// FIXME not good
			}
			return nextIFD - begin;
		}

		bool IFDFileContainer::locateDirsPreHook() 
		{ 
			return true;
		}


		bool
		IFDFileContainer::_locateDirs(void)
		{
			if(!locateDirsPreHook()) {
				return false;
			}
			Trace(DEBUG1) << "_locateDirs()\n";
			if (m_endian == ENDIAN_NULL) {
				char buf[4];
				m_file->seek(m_offset, SEEK_SET);
				m_file->read(buf, 4);
				m_endian = isMagicHeader(buf, 4);
				if (m_endian == ENDIAN_NULL) {
					// FIXME set error code
					return false;
				}
			}
			m_file->seek(m_offset + 4, SEEK_SET);
			int32_t offset = 0;
			readInt32(m_file, offset);
			m_dirs.clear();
			do {
				if (offset != 0) {
//					std::cerr.setf(std::ios_base::hex, std::ios_base::basefield);
					Trace(DEBUG1) << "push offset =0x" << offset << "\n";

					// we assume the offset is relative to the begining of
					// the IFD.
					IFDDir::Ref dir(new IFDDir(m_offset + offset,*this));
					m_dirs.push_back(dir);

//					std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);

					offset = dir->nextIFD();
				}
			} while(offset != 0);

			Trace(DEBUG1) << "# dir found = " << m_dirs.size() << "\n";
			return (m_dirs.size() != 0);
		}


	}
}

