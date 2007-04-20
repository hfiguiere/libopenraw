/*
 * libopenraw - jfifcontainer.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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



/**
 * @brief a JPEG container
 */


#ifndef _JFIFCONTAINER_H_
#define _JFIFCONTAINER_H_

#include <setjmp.h>
#include <cstdio>

namespace JPEG {
#include <jpeglib.h>
}

#include <libopenraw/types.h>
#include <libopenraw/consts.h>

#include "exception.h"
#include "rawcontainer.h"

namespace OpenRaw {

	class BitmapData;

	namespace Internals {

		class JFIFContainer
			: public RawContainer
		{
		public:
			JFIFContainer(IO::Stream *file, off_t offset);
			/** destructor */
			virtual ~JFIFContainer();

			bool getDimensions(uint32_t &x, uint32_t &y);
			bool getDecompressedData(BitmapData &data);

			/* libjpeg callbacks j_ is the prefix for these callbacks */
			static void j_init_source(JPEG::j_decompress_ptr cinfo);
			static JPEG::boolean j_fill_input_buffer(JPEG::j_decompress_ptr cinfo);
			static void j_skip_input_data(JPEG::j_decompress_ptr cinfo, 
																		long num_bytes);
//			static void j_jpeg_resync_to_restart(JPEG::j_decompress_ptr cinfo);
			static void j_term_source(JPEG::j_decompress_ptr cinfo);
			static void j_error_exit(JPEG::j_common_ptr cinfo);

		private:
		  int _loadHeader();

			struct JPEG::jpeg_decompress_struct m_cinfo;
			struct JPEG::jpeg_error_mgr m_jerr;
			jmp_buf m_jpegjmp;
			bool m_headerLoaded;
		};

	}
}

#endif
