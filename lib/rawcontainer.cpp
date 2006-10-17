/*
 * libopenraw - rawcontainer.cpp
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

#include "debug.h"
#include "endianutils.h"
#include "iofile.h"
#include "rawcontainer.h"



using namespace Debug;

namespace OpenRaw {
	namespace Internals {
	
	
		RawContainer::RawContainer(IOFile *file, off_t offset)
			: m_file(file),
				m_offset(offset),
				m_endian(ENDIAN_NULL)
		{
			m_file->open();
			m_file->seek(offset, SEEK_SET);
		}

	
		RawContainer::~RawContainer()
		{
			m_file->close();
		}


		bool 
		RawContainer::readInt16(IOFile *f, int16_t & v)
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
				v = EL16(buf);
			}
			else {
				v = BE16(buf);
			}
			std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);
			Trace(DEBUG1) << "value = " << v << "\n";
			return true;
		}


		bool 
		RawContainer::readInt32(IOFile *f, int32_t & v)
		{
			if (m_endian == ENDIAN_NULL) {

				Trace(ERROR) << "null endian\n";

				return false;
			}
			unsigned char buf[4];
			int s = f->read(buf, 4);
			if (s != 4) {
				Trace(ERROR) << "read " << s << " bytes\n";
				return false;
			}

			std::cerr.setf(std::ios_base::hex, std::ios_base::basefield);
			Trace(DEBUG1) << "read32 " << (int)buf[0] << " " << (int)buf [1] 
								<< " " << (int)buf [2] << " " << (int)buf[3] 
								<< "\n";

			if (m_endian == ENDIAN_LITTLE) {
				v = EL32(buf);
			}
			else {
				v = BE32(buf);
			}

			std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);
			Trace(DEBUG1) << "value = " << v << "\n";

			return true;
		}


		bool 
		RawContainer::readUInt16(IOFile *f, uint16_t & v)
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
			Trace(DEBUG1) << "readu16 " << (int)buf[0] << " " << (int)buf [1] 
								<< "\n";
			if (m_endian == ENDIAN_LITTLE) {
				v = EL16(buf);
			}
			else {
				v = BE16(buf);
			}
			std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);
			Trace(DEBUG1) << "value = " << v << "\n";
			return true;
		}


		bool 
		RawContainer::readUInt32(IOFile *f, uint32_t & v)
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
			Trace(DEBUG1) << "readu32 " << (int)buf[0] << " " << (int)buf [1] 
								<< " " << (int)buf [2] << " " << (int)buf[3] 
								<< "\n";

			if (m_endian == ENDIAN_LITTLE) {
				v = EL32(buf);
			}
			else {
 				v = BE32(buf);
			}

			std::cerr.setf((std::ios_base::fmtflags)0, std::ios_base::basefield);
			Trace(DEBUG1) << "value = " << v << "\n";

			return true;
		}


		size_t 
		RawContainer::fetchData(void *buf, const off_t offset,
														const size_t buf_size)
		{
			size_t s;
			m_file->seek(offset, SEEK_SET);
			s = m_file->read(buf, buf_size);
			return s;
		}


	}
}
