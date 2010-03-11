/*
 * libopenraw - rawcontainer.cpp
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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

#include <libopenraw/types.h>

#include "trace.h"
#include "endianutils.h"
#include "io/file.h"
#include "rawcontainer.h"



using namespace Debug;

namespace OpenRaw {
	namespace Internals {
	
	
		RawContainer::RawContainer(IO::Stream *_file, off_t offset)
			: m_file(_file),
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


		bool RawContainer::readInt8(IO::Stream *f, int8_t & v)
		{
			unsigned char buf;
			int s = f->read(&buf, 1);
			if (s != 1) {
				return false;
			}
			v = buf;
			return true;
		}

		bool RawContainer::readUInt8(IO::Stream *f, uint8_t & v)
		{
			unsigned char buf;
			int s = f->read(&buf, 1);
			if (s != 1) {
				return false;
			}
			v = buf;
			return true;
		}

		bool 
		RawContainer::readInt16(IO::Stream *f, int16_t & v)
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
			if (m_endian == ENDIAN_LITTLE) {
				v = EL16(buf);
			}
			else {
				v = BE16(buf);
			}
			return true;
		}


		bool 
		RawContainer::readInt32(IO::Stream *f, int32_t & v)
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

			if (m_endian == ENDIAN_LITTLE) {
				v = EL32(buf);
			}
			else {
				v = BE32(buf);
			}

			return true;
		}


		bool 
		RawContainer::readUInt16(IO::Stream *f, uint16_t & v)
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
			if (m_endian == ENDIAN_LITTLE) {
				v = EL16(buf);
			}
			else {
				v = BE16(buf);
			}
			return true;
		}


		bool 
		RawContainer::readUInt32(IO::Stream *f, uint32_t & v)
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

			if (m_endian == ENDIAN_LITTLE) {
				v = EL32(buf);
			}
			else {
 				v = BE32(buf);
			}

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
