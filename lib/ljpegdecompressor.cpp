/*
 * libopenraw - ljpegdecompressor.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <boost/scoped_ptr.hpp>

#include <libopenraw++/bitmapdata.h>
#include "io/memstream.h"
#include "debug.h"
#include "jfifcontainer.h"
#include "ljpegdecompressor.h"

namespace OpenRaw {

	class BitmapData;
		
	namespace Internals {

		LJpegDecompressor::LJpegDecompressor(const BitmapData *data)
			: Decompressor(data)
		{
		}


		LJpegDecompressor::~LJpegDecompressor()
		{
		}
		
		
		BitmapData *LJpegDecompressor::decompress()
		{
			boost::scoped_ptr<IO::Stream> stream(new IO::MemStream(m_data->data(),
																														 m_data->size()));
			boost::scoped_ptr<JFIFContainer> container(new JFIFContainer(stream.get(), 0));

			BitmapData *bitmap = new BitmapData();
			bitmap->setDimensions(m_data->x(), m_data->y());
			bitmap->setDataType(OR_DATA_TYPE_CFA);
			
			container->getDecompressedData(*bitmap);

			return bitmap;
		}

	}
}
