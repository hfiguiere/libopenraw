/*
 * libopenraw - ljpegdecompressor.h
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

#ifndef __OPENRAW_LJPEGDECOMPRESSOR_H__
#define __OPENRAW_LJPEGDECOMPRESSOR_H__

#include <stdint.h>

#include <vector>

#include <libopenraw/libopenraw.h>

#include "exception.h"
#include "decompressor.h"

namespace OpenRaw {

	class RawData;
		
	namespace Internals {
		struct HuffmanTable;
		struct DecompressInfo;
		typedef int16_t ComponentType;
		typedef ComponentType *MCU;


		class LJpegDecompressor
			: public Decompressor
		{
		public:
			LJpegDecompressor(IO::Stream *,
												RawContainer *);
			virtual ~LJpegDecompressor();
			
			/** decompress the bitmapdata and return a new bitmap
			 * @param in a preallocated BitmapData instance
			 * or NULL if decompress has to allocate it.
			 * @return the new bitmap decompressed. NULL is failure.
			 * Caller owns it.
			 * @todo use a shared_ptr here, or something
			 */
			virtual RawData *decompress(RawData *in = NULL);
			void setSlices(const std::vector<uint16_t> & slices, 
										 std::vector<uint16_t>::size_type idx = 0);
			bool isSliced() const
				{ 
					return m_slices.size() > 1; 
				}
		private:
			
			/** read the bits
			 * @param s the stream to read from
			 * @param bitCount the number of bit
			 * @return the value
			 */
			int32_t readBits(IO::Stream * s, uint16_t bitCount);
			int32_t show_bits8(IO::Stream * s);
			void flush_bits(uint16_t nbits);
			int32_t get_bits(uint16_t nbits);
			int32_t get_bit();
			
/** 
 * Enumerate all the JPEG marker codes
 */
			typedef enum {
				M_SOF0 = 0xc0,
				M_SOF1 = 0xc1,
				M_SOF2 = 0xc2,
				M_SOF3 = 0xc3,
				
				M_SOF5 = 0xc5,
				M_SOF6 = 0xc6,
				M_SOF7 = 0xc7,

				M_JPG = 0xc8,
				M_SOF9 = 0xc9,
				M_SOF10 = 0xca,
				M_SOF11 = 0xcb,

				M_SOF13 = 0xcd,
				M_SOF14 = 0xce,
				M_SOF15 = 0xcf,

				M_DHT = 0xc4,

				M_DAC = 0xcc,

				M_RST0 = 0xd0,
				M_RST1 = 0xd1,
				M_RST2 = 0xd2,
				M_RST3 = 0xd3,
				M_RST4 = 0xd4,
				M_RST5 = 0xd5,
				M_RST6 = 0xd6,
				M_RST7 = 0xd7,

				M_SOI = 0xd8,
				M_EOI = 0xd9,
				M_SOS = 0xda,
				M_DQT = 0xdb,
				M_DNL = 0xdc,
				M_DRI = 0xdd,
				M_DHP = 0xde,
				M_EXP = 0xdf,

				M_APP0 = 0xe0,
				M_APP15 = 0xef,

				M_JPG0 = 0xf0,
				M_JPG13 = 0xfd,
				M_COM = 0xfe,

				M_TEM = 0x01,

				M_ERROR = 0x100
			} JpegMarker;

			void DecoderStructInit (DecompressInfo *dcPtr) throw(DecodingException);
			void HuffDecoderInit (DecompressInfo *dcPtr) throw(DecodingException);
			void ProcessRestart (DecompressInfo *dcPtr) throw(DecodingException);
			void DecodeFirstRow(DecompressInfo *dcPtr,
													MCU *curRowBuf);
			void DecodeImage(DecompressInfo *dcPtr);
			int32_t QuickPredict(int32_t col, int16_t curComp,
													 MCU *curRowBuf, MCU *prevRowBuf,
													 int32_t psv);
			void PmPutRow(MCU* RowBuf, int32_t numComp, int32_t numCol, int32_t Pt);
			void GetDht (DecompressInfo *dcPtr) throw(DecodingException);
			void GetDri (DecompressInfo *dcPtr) throw(DecodingException);
			void GetSof (DecompressInfo *dcPtr) throw(DecodingException);
			void GetSos (DecompressInfo *dcPtr) throw(DecodingException);
			JpegMarker ProcessTables (DecompressInfo *dcPtr);
			void ReadFileHeader (DecompressInfo *dcPtr) throw(DecodingException);
			int32_t ReadScanHeader (DecompressInfo *dcPtr);
			int32_t HuffDecode(HuffmanTable *htbl);

			std::vector<uint16_t> m_slices;

			MCU *m_mcuROW1, *m_mcuROW2;
			char *m_buf1,*m_buf2;

			/** fill the bit buffer */
			void fillBitBuffer (IO::Stream * s, uint16_t nbits);
			uint16_t m_bitsLeft;
			uint32_t m_getBuffer;
			RawData *m_output;

			/** private copy constructor to make sure it is not called */
			LJpegDecompressor(const LJpegDecompressor& f);
			/** private = operator to make sure it is never called */
			LJpegDecompressor & operator=(const LJpegDecompressor&);
		};

	}
}



#endif
