/* -*- tab-width:4; c-basic-offset:4 indent-tabs-mode:t -*- */
/*
 * libopenraw - rafcontainer.cpp
 *
 * Copyright (C) 2011-2016 Hubert Figuiere
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

#include <fcntl.h>
#include <string.h>
#include <memory>

#include "rafcontainer.hpp"
#include "raffile.hpp"
#include "jfifcontainer.hpp"
#include "ifdfilecontainer.hpp"
#include "rafmetacontainer.hpp"
#include "io/stream.hpp"
#include "io/streamclone.hpp"

namespace OpenRaw {
namespace Internals {

RafContainer::RafContainer(const IO::Stream::Ptr &_file)
	: RawContainer(_file, 0)
	, m_read(false)
	, m_version(0)
	, m_jpegPreview(nullptr)
	, m_cfaContainer(nullptr)
	, m_metaContainer(nullptr)
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
			m_cfaContainer = new IfdFileContainer(
				std::make_shared<IO::StreamClone>(
                    m_file, m_offsetDirectory.cfaOffset), 0);
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
			m_jpegPreview = new JfifContainer(
				std::make_shared<IO::StreamClone>(
                    m_file, m_offsetDirectory.jpegOffset), 0);
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
			m_metaContainer = new RafMetaContainer(
				std::make_shared<IO::StreamClone>(
                    m_file, m_offsetDirectory.metaOffset));
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
	auto result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_version = result.unwrap();

	m_file->seek(20, SEEK_CUR);

	result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_offsetDirectory.jpegOffset = result.unwrap();
	result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_offsetDirectory.jpegLength = result.unwrap();
	result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_offsetDirectory.metaOffset = result.unwrap();
	result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_offsetDirectory.metaLength = result.unwrap();
	result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_offsetDirectory.cfaOffset = result.unwrap();
	result = readUInt32(m_file);
	if (result.empty()) {
		return false;
	}
	m_offsetDirectory.cfaLength = result.unwrap();

	return true;
}

}
}
