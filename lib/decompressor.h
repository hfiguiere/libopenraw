/*
 * libopenraw - decompressor.h
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

#ifndef __OPENRAW_DECOMPRESS_H__
#define __OPENRAW_DECOMPRESS_H__

#include <boost/noncopyable.hpp>

#include <libopenraw/libopenraw.h>


namespace OpenRaw {

	class RawData;

	namespace IO {
		class Stream;
	}

	namespace Internals {

		class RawContainer;

		class Decompressor
			: private boost::noncopyable
		{
		public:
			Decompressor(IO::Stream * stream,
									 RawContainer * container);
			virtual ~Decompressor();
			
			/** decompress the bitmapdata and return a new bitmap
			 * @param in a preallocated BitmapData instance
			 * or NULL if decompress has to allocate it.
			 * @return the new bitmap decompressed. NULL is failure.
			 * Caller owns it.
			 * @todo use a shared_ptr here, or something
			 */
			virtual RawData *decompress(RawData *in = NULL) = 0;
		protected:
			IO::Stream *m_stream;
			RawContainer *m_container;

			/** private copy constructor to make sure it is not called */
			Decompressor(const Decompressor& f);
			/** private = operator to make sure it is never called */
			Decompressor & operator=(const Decompressor&);
		};

	}
}



#endif
