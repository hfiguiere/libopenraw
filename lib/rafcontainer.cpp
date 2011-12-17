/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - rafcontainer.cpp
 *
 * Copyright (C) 2011 Hubert Figuiere
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

#include <string.h>

#include "rafcontainer.h"
#include "raffile.h"
#include "jfifcontainer.h"
#include "ifdfilecontainer.h"
#include "rafmetacontainer.h"
#include "endianutils.h"
#include "io/stream.h"
#include "io/streamclone.h"

namespace OpenRaw {
namespace Internals {
	

RafContainer::RafContainer(IO::Stream *_file)
	: RawContainer(_file, 0)
	, m_read(false)
	, m_version(0)
	, m_jpegPreview(NULL)
	, m_cfaContainer(NULL)
	, m_metaContainer(NULL)
{
	memset((void*)&m_offsetDirectory, 0, sizeof(m_offsetDirectory));
}

RafContainer::~RafContainer()
{
	delete m_jpegPreview;
	delete m_cfaContainer;
	delete m_metaContainer;
}

const std::string & RafContainer::getModel()
{
	if(!m_read) {
		_readHeader();
	}
	return m_model;
}

IfdFileContainer * RafContainer::getCfaContainer()
{
	if(!m_cfaContainer) {
		if(!m_read) {
			_readHeader();
		}
		if(m_offsetDirectory.cfaOffset && m_offsetDirectory.cfaLength) {
			m_cfaContainer = new IfdFileContainer(new IO::StreamClone(m_file, m_offsetDirectory.cfaOffset), 0);
		}		
	}
	return m_cfaContainer;
}
	
JfifContainer * RafContainer::getJpegPreview()
{
	if(!m_jpegPreview) {
		if(!m_read) {
			_readHeader();
		}
		if(m_offsetDirectory.jpegOffset && m_offsetDirectory.jpegLength) {
			m_jpegPreview = new JfifContainer(new IO::StreamClone(m_file, m_offsetDirectory.jpegOffset), 0);
		}
	}
	return m_jpegPreview;
}
	
RafMetaContainer * RafContainer::getMetaContainer()
{
	if(!m_metaContainer) {
		if(!m_read) {
			_readHeader();
		}
		if(m_offsetDirectory.metaOffset && m_offsetDirectory.metaLength) {
			m_metaContainer = new RafMetaContainer(new IO::StreamClone(m_file, m_offsetDirectory.metaOffset));
		}
	}
	return m_metaContainer;
}

bool RafContainer::_readHeader()
{
	char magic[29];
	char model[33];
	magic[28] = 0;
	model[32] = 0;	
	m_read = true;

	m_file->read(magic, 28);

	if(strncmp(magic, RAF_MAGIC, RAF_MAGIC_LEN) != 0) {
		// not a RAF file
		return false;
	}
	
	setEndian(ENDIAN_BIG);
	
	m_file->read(model, 32);
	m_model = model;
	readUInt32(m_file, m_version);
	m_file->seek(20, SEEK_CUR);
	readUInt32(m_file, m_offsetDirectory.jpegOffset);
	readUInt32(m_file, m_offsetDirectory.jpegLength);
	readUInt32(m_file, m_offsetDirectory.metaOffset);
	readUInt32(m_file, m_offsetDirectory.metaLength);
	readUInt32(m_file, m_offsetDirectory.cfaOffset);
	readUInt32(m_file, m_offsetDirectory.cfaLength);
	
	return true;
}

}
}
