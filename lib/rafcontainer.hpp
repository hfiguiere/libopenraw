/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - rafcontainer.h
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

#ifndef OR_INTERNALS_RAFCONTAINER_H_
#define OR_INTERNALS_RAFCONTAINER_H_

#include <stdint.h>
#include <string>

#include "io/stream.hpp"
#include "rawcontainer.hpp"

namespace OpenRaw {

namespace Internals {

class JfifContainer;
class IfdFileContainer;
class RafMetaContainer;
	
struct RafOffsetDirectory
{
	// 36 bytes skipped
	uint32_t jpegOffset;
	uint32_t jpegLength;
	uint32_t metaOffset;
	uint32_t metaLength;
	uint32_t cfaOffset;
	uint32_t cfaLength;
};
	
class RafContainer
	: public RawContainer
{
public:
	RafContainer(const IO::Stream::Ptr &_file);
	/** destructor */
	virtual ~RafContainer();

	const std::string & getModel();
	JfifContainer * getJpegPreview();
	IfdFileContainer * getCfaContainer();
	RafMetaContainer * getMetaContainer();
	uint32_t getJpegOffset() const
	{
		return m_offsetDirectory.jpegOffset;
	}
	uint32_t getJpegLength() const
	{
		return m_offsetDirectory.jpegLength;
	}
	uint32_t getCfaOffset() const
	{
		return m_offsetDirectory.cfaOffset;
	}
	uint32_t getCfaLength() const
	{
		return m_offsetDirectory.cfaLength;
	}
private:
	bool _readHeader();
	bool m_read;
	std::string m_model;
	uint32_t m_version;
	RafOffsetDirectory m_offsetDirectory;
	
	JfifContainer * m_jpegPreview;
	IfdFileContainer * m_cfaContainer;
	RafMetaContainer * m_metaContainer;
};

}
}

#endif
