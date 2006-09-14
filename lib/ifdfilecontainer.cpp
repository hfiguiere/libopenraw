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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sys/types.h>

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>

#include "debug.h"

#include "ifdfilecontainer.h"
#include "iofile.h"


using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		IFDFileContainer::IFDFileContainer(IOFile *file, off_t offset)
			: RawContainer(file, offset), 
				m_error(0),
				m_endian(ENDIAN_NULL),
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
			Int32 nextIFD;
			readInt32(m_file, nextIFD);
			Trace(DEBUG1) << "nextIFD = " << nextIFD << "\n";
			if (nextIFD == 0) {
				// FIXME not good
			}
			return nextIFD - begin;
		}

		size_t 
		IFDFileContainer::fetchData(void *buf, const off_t offset,
																const size_t buf_size)
		{
			size_t s;
			m_file->seek(offset, SEEK_SET);
			s = m_file->read(buf, buf_size);
			return s;
		}


		bool 
		IFDFileContainer::readInt16(IOFile *f, Int16 & v)
		{
			if (m_endian == ENDIAN_NULL) {

				Trace(ERROR) << "null endian\n";

				return false;
			}
			unsigned char buf[2];
			int s = f->read(buf, 2);
			if (s != 2) {
				return false;
			}
			std::cerr.setf(std::ios_base::hex, std::ios_base::basefield);
			Trace(DEBUG1) << "read16 " << (int)buf[0] << " " << (int)buf [1] 
								<< "\n";
			if (m_endian == ENDIAN_LITTLE) {
				v = buf[0] | (buf[1] << 8);
			}
			else {
				v = buf[1] | (buf[0] << 8);
			}
			std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);
			Trace(DEBUG1) << "value = " << v << "\n";
			return true;
		}


		bool 
		IFDFileContainer::readInt32(IOFile *f, Int32 & v)
		{
			if (m_endian == ENDIAN_NULL) {

				Trace(ERROR) << "null endian\n";

				return false;
			}
			unsigned char buf[4];
			int s = f->read(buf, 4);
			if (s != 4) {
				return false;
			}

			std::cerr.setf(std::ios_base::hex, std::ios_base::basefield);
			Trace(DEBUG1) << "read32 " << (int)buf[0] << " " << (int)buf [1] 
								<< " " << (int)buf [2] << " " << (int)buf[3] 
								<< "\n";

			if (m_endian == ENDIAN_LITTLE) {
				v = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
			}
			else {
 				v = buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
			}

			std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);
			Trace(DEBUG1) << "value = " << v << "\n";

			return true;
		}


		bool
		IFDFileContainer::_locateDirs(void)
		{
			Trace(DEBUG1) << "_locateDirs()\n";
			if (m_endian == ENDIAN_NULL) {
				char buf[4];
				m_file->read(buf, 4);
				m_endian = isMagicHeader(buf, 4);
				if (m_endian == ENDIAN_NULL) {
					// FIXME set error code
					return false;
				}
			}
			m_file->seek(4, SEEK_SET);
			off_t offset = 0;
			readInt32(m_file, (Int32&)offset);
			m_dirs.clear();
			do {
				if (offset != 0) {
					std::cerr.setf(std::ios_base::hex, std::ios_base::basefield);
					Trace(DEBUG1) << "push offset =0x" << offset << "\n";

					IFDDir::Ref dir(new IFDDir(offset,*this));
					m_dirs.push_back(dir);

					std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);

					offset = dir->nextIFD();
				}
			} while(offset != 0);

			Trace(DEBUG1) << "# dir found = " << m_dirs.size() << "\n";
			return (m_dirs.size() != 0);
		}


	}
}

