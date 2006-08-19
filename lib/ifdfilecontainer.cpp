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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include <vector>

#include "ifdfilecontainer.h"
#include "iofile.h"



namespace OpenRaw {
	namespace Internals {


	IFDFileContainer::IFDFileContainer(IOFile *file, off_t offset)
		: RawContainer(file, offset), 
		  m_error(0),
		  m_currentDir(-1),
		  m_numDirs(-1),
		  m_numTags(-1),
		  m_endian(ENDIAN_NULL)
	{
	}
	
	IFDFileContainer::~IFDFileContainer()
	{

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



	bool 
	IFDFileContainer::SetDirectory(int dir)
	{
		bool ret = true;
		if (m_numDirs == -1) {
			// FIXME check result
			ret = _locateDirs();
			if (!ret) {
				return ret;
			}
		}
		if (dir > m_dirsOffsets.size()) {
			// FIXME set error
			return false;
		}
		off_t offset = m_dirsOffsets[dir];
		// FIXME check error
		m_file->seek(offset, SEEK_SET);
		m_currentDir = dir;
		ReadInt16(m_file, m_numTags);
		m_file->seek(offset, SEEK_SET);
		return ret;
	}


	bool 
	IFDFileContainer::ReadInt16(IOFile *f, Int16 & v)
	{
		if (m_endian == ENDIAN_NULL) {
			return false;
		}
		char buf[2];
		int s = f->read(buf, 2);
		if (s != 2) {
			return false;
		}
		if (m_endian == ENDIAN_LITTLE) {
			v = buf[1] | (buf[0] << 8);
		}
		else {
			v = buf[0] | (buf[1] << 8);
		}
		return true;
	}


	bool 
	IFDFileContainer::ReadInt32(IOFile *f, Int32 & v)
	{
		if (m_endian == ENDIAN_NULL) {
			return false;
		}
		char buf[4];
		int s = f->read(buf, 4);
		if (s != 4) {
			return false;
		}
		if (m_endian == ENDIAN_LITTLE) {
			v = buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
		}
		else {
			v = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
		}
		return true;
	}


	bool
	IFDFileContainer::_locateDirs(void)
	{
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
		m_dirsOffsets.clear();
		do {
			if (ReadInt32(m_file, (Int32&)offset) && (offset != 0)) {
				m_dirsOffsets.push_back(offset);
			}
		} while(offset != 0);
		return (m_dirsOffsets.size() != 0);
	}


#if 0
#include <tiffio.h>

/* These are the glue routing for TIFFIO */
	static tsize_t
	_TIFFRead(thandle_t fileRef, tdata_t data, tsize_t size)
	{
		or_debug("_TIFFRead()");
		return static_cast<RawFile*>(fileRef)->read(data, size);
	}

	static tsize_t
	_TIFFWrite(thandle_t fileRef, tdata_t data, tsize_t size)
	{
		or_debug("_TIFFWrite()");
		return -1;
	}


	static toff_t
	_TIFFSeek(thandle_t fileRef, toff_t off, int offmode)
	{
		or_debug("_TIFFSeek()");
		return static_cast<RawFile*>(fileRef)->seek(off, offmode);
	}

	static int
	_TIFFClose(thandle_t fileRef)
	{
		or_debug("_TIFFClose()");
		/* this is a no op*/
		return 0;
	}


	static toff_t
	_TIFFSize(thandle_t fileRef)
	{
		or_debug("_TIFFSize()");
	
		return static_cast<RawFile*>(fileRef)->filesize();
	}


	static int
	_TIFFMapFile(thandle_t fileRef, tdata_t* d, toff_t* s)
	{
		or_debug("_TIFFMapFile()");
		toff_t size = _TIFFSize(fileRef);
	
		if (size != -1) {
			*d = static_cast<RawFile*>(fileRef)->mmap(size, 0);
			if (*d != (tdata_t) -1) {
				*s = size;
				return 1;
			}
		}
	
		return 0;
	}
	static void
	_TIFFUnmapFile(thandle_t fileRef, tdata_t d, toff_t o)
	{
		or_debug("_TIFFUnmapFile()");
		static_cast<RawFile*>(fileRef)->munmap(d, o);
	}


	::TIFF *TIFF::raw_tiff_open()
	{
		return TIFFClientOpen("", "r", (thandle_t)this,
							  &_TIFFRead, &_TIFFWrite, &_TIFFSeek, &_TIFFClose,
							  &_TIFFSize, &_TIFFMapFile, &_TIFFUnmapFile);
	}


};

#endif


}

}

