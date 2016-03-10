/* -*- Mode: C++ -*- */
/*
 * libopenraw - orfcontainer.h
 *
 * Copyright (C) 2006-2015 Hubert Figuiere
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


#ifndef OR_INTERNALS_ORF_CONTAINER_H_
#define OR_INTERNALS_ORF_CONTAINER_H_

#include <sys/types.h>

#include "io/stream.hpp"
#include "rawcontainer.hpp"
#include "ifdfilecontainer.hpp"

namespace OpenRaw {
namespace Internals {

static const char ORF_SUBTYPE_16BPP = 'O';
static const char ORF_SUBTYPE_12BPP = 'S';

class OrfContainer
	: public IfdFileContainer
{
public:
	OrfContainer(const IO::Stream::Ptr &file, off_t offset);
	/** destructor */
	virtual ~OrfContainer();

  	OrfContainer(const OrfContainer &) = delete;
	OrfContainer & operator=(const OrfContainer &) = delete;

	/**
         * Check the ORF magic header.
	 */
	virtual IfdFileContainer::EndianType
	isMagicHeader(const char *p, int len) override;

private:
	char subtype_;
};

}
}


#endif
