/*
 * libopenraw - ciffcontainertest.cpp
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

#include "debug.h"
#include "ciffcontainer.h"
#include "io/file.h"

using namespace OpenRaw::Internals;

int main(int argc, char **argv)
{
	Debug::Trace::setDebugLevel(Debug::DEBUG2);

	OpenRaw::IO::File file(argv[1]);
	CIFFContainer container(&file);
	
	const CIFF::HeapFileHeader & hdr = container.header();

	std::cout << "byteOrder = " << hdr.byteOrder[0] << hdr.byteOrder[1] << std::endl;

	CIFF::Heap::Ref heap = container.heap();
	std::vector<CIFF::RecordEntry> & records = heap->records();

	std::cout << "vector size " << records.size() << std::endl;
}

