/*
 * libopenraw - ifdfilecontainer.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
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

#include <sys/types.h>

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>

#include "trace.h"

#include "ifdfilecontainer.h"
#include "io/file.h"


using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		IfdFileContainer::IfdFileContainer(IO::Stream *_file, off_t _offset)
			: RawContainer(_file, _offset), 
			  m_error(0),
			  m_exif_offset_correction(0),
			  m_current_dir(),
			  m_dirs()
		{
		}
	
		IfdFileContainer::~IfdFileContainer()
		{
			m_dirs.clear();
		}


		IfdFileContainer::EndianType 
		IfdFileContainer::isMagicHeader(const char *p, int len)
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


		int IfdFileContainer::countDirectories(void)
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

		std::vector<IfdDir::Ref> & 
		IfdFileContainer::directories()
		{
			if (m_dirs.size() == 0) {
				countDirectories();
			}
			return m_dirs;
		}

		IfdDir::Ref
		IfdFileContainer::setDirectory(int dir)
		{
			if (dir < 0) {
				// FIXME set error
				return IfdDir::Ref();
			}
			// FIXME handle negative values
			int n = countDirectories();
			if (n <= 0) {
				// FIXME set error
				return IfdDir::Ref();
			}
			// dir is signed here because we can pass negative 
			// value for specific Exif IFDs.
			if (dir > (int)m_dirs.size()) {
				// FIXME set error
				return IfdDir::Ref();
			}
			m_current_dir = m_dirs[dir];
			m_current_dir->load();
			return m_current_dir;
		}


		size_t 
		IfdFileContainer::getDirectoryDataSize()
		{
			// TODO move to IFDirectory
			Trace(DEBUG1) << "getDirectoryDataSize()" << "\n";
			off_t dir_offset = m_current_dir->offset();
			// FIXME check error
			Trace(DEBUG1) << "offset = " << dir_offset 
								<< " m_numTags = " << m_current_dir->numTags() << "\n";
			off_t begin = dir_offset + 2 + (m_current_dir->numTags()*12);
			
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

		bool IfdFileContainer::locateDirsPreHook() 
		{ 
			return true;
		}


		bool
		IfdFileContainer::_locateDirs(void)
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
			int32_t dir_offset = 0;
			readInt32(m_file, dir_offset);
			m_dirs.clear();
			do {
				if (dir_offset != 0) {
//					std::cerr.setf(std::ios_base::hex, std::ios_base::basefield);
					Trace(DEBUG1) << "push offset =0x" << dir_offset << "\n";

					// we assume the offset is relative to the begining of
					// the IFD.
					IfdDir::Ref dir(new IfdDir(m_offset + dir_offset,*this));
					m_dirs.push_back(dir);

//					std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);

					dir_offset = dir->nextIFD();
				}
			} while(dir_offset != 0);

			Trace(DEBUG1) << "# dir found = " << m_dirs.size() << "\n";
			return (m_dirs.size() != 0);
		}


	}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/

