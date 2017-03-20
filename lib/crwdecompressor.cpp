/*
 * libopenraw - crwdecompressor.h
 *
 * Copyright (C) 2007-2016 Hubert Figuiere
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
/*
	Simple reference decompresser for Canon digital cameras.
	Outputs raw 16-bit CCD data, no header, native byte order.

	Written by Dave Coffin.
	Downloaded from http://cybercom.net/~dcoffin/dcraw/decompress.c

	$Revision: 1.1 $
	$Date: 2005/06/27 14:07:24 $
*/

#include <fcntl.h>
#include <string.h>

#include <libopenraw/consts.h>
#include <libopenraw/debug.h>

#include "rawdata.hpp"
#include "crwdecompressor.hpp"
#include "exception.hpp"
#include "trace.hpp"
#include "io/stream.hpp"

namespace OpenRaw {	namespace Internals {

CrwDecompressor::CrwDecompressor(IO::Stream * stream,
                                 RawContainer * container)
    : Decompressor(stream, container),
      m_table(0),
      m_height(0), m_width(0),
      m_free(0), m_leaf(0),
      m_bitbuf(0), m_vbits(0)
{
}


CrwDecompressor::~CrwDecompressor()
{
}


/*
  A rough description of Canon's compression algorithm:

  +  Each pixel outputs a 10-bit sample, from 0 to 1023.
  +  Split the data into blocks of 64 samples each.
  +  Subtract from each sample the value of the sample two positions
  to the left, which has the same color filter.  From the two
  leftmost samples in each row, subtract 512.
  +  For each nonzero sample, make a token consisting of two four-bit
  numbers.  The low nibble is the number of bits required to
  represent the sample, and the high nibble is the number of
  zero samples preceding this sample.
  +  Output this token as a variable-length bitstring using
  one of three tablesets.  Follow it with a fixed-length
  bitstring containing the sample.

  The "first_decode" table is used for the first sample in each
  block, and the "second_decode" table is used for the others.
*/

/*
  Construct a decode tree according the specification in *source.
  The first 16 bytes specify how many codes should be 1-bit, 2-bit
  3-bit, etc.  Bytes after that are the leaf values.

  For example, if the source is

  { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
  0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

  then the code is

  00		0x04
  010		0x03
  011		0x05
  100		0x06
  101		0x02
  1100		0x07
  1101		0x01
  11100		0x08
  11101		0x09
  11110		0x00
  111110		0x0a
  1111110		0x0b
  1111111		0xff
*/
void CrwDecompressor::make_decoder(decode_t *dest, const uint8_t *source,
                                   int level)
{
    int i, next;

    if (level==0) {
        m_free = dest;
        m_leaf = 0;
    }
    m_free++;
/*
  At what level should the next leaf appear?
*/
    for (i=next=0; i <= m_leaf && next < 16; ) {
        i += source[next++];
    }

    if (i > m_leaf) {
        if (level < next) {		/* Are we there yet? */
            dest->branch[0] = m_free;
            make_decoder(m_free,source,level+1);
            dest->branch[1] = m_free;
            make_decoder(m_free,source,level+1);
        }
        else {
            dest->leaf = source[16 + m_leaf++];
        }
    }
}

void CrwDecompressor::init_tables(uint32_t table_idx)
{
    static const uint8_t first_tree[3][29] = {
        { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
          0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

        { 0,2,2,3,1,1,1,1,2,0,0,0,0,0,0,0,
          0x03,0x02,0x04,0x01,0x05,0x00,0x06,0x07,0x09,0x08,0x0a,0x0b,0xff  },

        { 0,0,6,3,1,1,2,0,0,0,0,0,0,0,0,0,
          0x06,0x05,0x07,0x04,0x08,0x03,0x09,0x02,0x00,0x0a,0x01,0x0b,0xff  },
    };

    static const uint8_t second_tree[3][180] = {
        { 0,2,2,2,1,4,2,1,2,5,1,1,0,0,0,139,
          0x03,0x04,0x02,0x05,0x01,0x06,0x07,0x08,
          0x12,0x13,0x11,0x14,0x09,0x15,0x22,0x00,0x21,0x16,0x0a,0xf0,
          0x23,0x17,0x24,0x31,0x32,0x18,0x19,0x33,0x25,0x41,0x34,0x42,
          0x35,0x51,0x36,0x37,0x38,0x29,0x79,0x26,0x1a,0x39,0x56,0x57,
          0x28,0x27,0x52,0x55,0x58,0x43,0x76,0x59,0x77,0x54,0x61,0xf9,
          0x71,0x78,0x75,0x96,0x97,0x49,0xb7,0x53,0xd7,0x74,0xb6,0x98,
          0x47,0x48,0x95,0x69,0x99,0x91,0xfa,0xb8,0x68,0xb5,0xb9,0xd6,
          0xf7,0xd8,0x67,0x46,0x45,0x94,0x89,0xf8,0x81,0xd5,0xf6,0xb4,
          0x88,0xb1,0x2a,0x44,0x72,0xd9,0x87,0x66,0xd4,0xf5,0x3a,0xa7,
          0x73,0xa9,0xa8,0x86,0x62,0xc7,0x65,0xc8,0xc9,0xa1,0xf4,0xd1,
          0xe9,0x5a,0x92,0x85,0xa6,0xe7,0x93,0xe8,0xc1,0xc6,0x7a,0x64,
          0xe1,0x4a,0x6a,0xe6,0xb3,0xf1,0xd3,0xa5,0x8a,0xb2,0x9a,0xba,
          0x84,0xa4,0x63,0xe5,0xc5,0xf3,0xd2,0xc4,0x82,0xaa,0xda,0xe4,
          0xf2,0xca,0x83,0xa3,0xa2,0xc3,0xea,0xc2,0xe2,0xe3,0xff,0xff  },

        { 0,2,2,1,4,1,4,1,3,3,1,0,0,0,0,140,
          0x02,0x03,0x01,0x04,0x05,0x12,0x11,0x06,
          0x13,0x07,0x08,0x14,0x22,0x09,0x21,0x00,0x23,0x15,0x31,0x32,
          0x0a,0x16,0xf0,0x24,0x33,0x41,0x42,0x19,0x17,0x25,0x18,0x51,
          0x34,0x43,0x52,0x29,0x35,0x61,0x39,0x71,0x62,0x36,0x53,0x26,
          0x38,0x1a,0x37,0x81,0x27,0x91,0x79,0x55,0x45,0x28,0x72,0x59,
          0xa1,0xb1,0x44,0x69,0x54,0x58,0xd1,0xfa,0x57,0xe1,0xf1,0xb9,
          0x49,0x47,0x63,0x6a,0xf9,0x56,0x46,0xa8,0x2a,0x4a,0x78,0x99,
          0x3a,0x75,0x74,0x86,0x65,0xc1,0x76,0xb6,0x96,0xd6,0x89,0x85,
          0xc9,0xf5,0x95,0xb4,0xc7,0xf7,0x8a,0x97,0xb8,0x73,0xb7,0xd8,
          0xd9,0x87,0xa7,0x7a,0x48,0x82,0x84,0xea,0xf4,0xa6,0xc5,0x5a,
          0x94,0xa4,0xc6,0x92,0xc3,0x68,0xb5,0xc8,0xe4,0xe5,0xe6,0xe9,
          0xa2,0xa3,0xe3,0xc2,0x66,0x67,0x93,0xaa,0xd4,0xd5,0xe7,0xf8,
          0x88,0x9a,0xd7,0x77,0xc4,0x64,0xe2,0x98,0xa5,0xca,0xda,0xe8,
          0xf3,0xf6,0xa9,0xb2,0xb3,0xf2,0xd2,0x83,0xba,0xd3,0xff,0xff  },

        { 0,0,6,2,1,3,3,2,5,1,2,2,8,10,0,117,
          0x04,0x05,0x03,0x06,0x02,0x07,0x01,0x08,
          0x09,0x12,0x13,0x14,0x11,0x15,0x0a,0x16,0x17,0xf0,0x00,0x22,
          0x21,0x18,0x23,0x19,0x24,0x32,0x31,0x25,0x33,0x38,0x37,0x34,
          0x35,0x36,0x39,0x79,0x57,0x58,0x59,0x28,0x56,0x78,0x27,0x41,
          0x29,0x77,0x26,0x42,0x76,0x99,0x1a,0x55,0x98,0x97,0xf9,0x48,
          0x54,0x96,0x89,0x47,0xb7,0x49,0xfa,0x75,0x68,0xb6,0x67,0x69,
          0xb9,0xb8,0xd8,0x52,0xd7,0x88,0xb5,0x74,0x51,0x46,0xd9,0xf8,
          0x3a,0xd6,0x87,0x45,0x7a,0x95,0xd5,0xf6,0x86,0xb4,0xa9,0x94,
          0x53,0x2a,0xa8,0x43,0xf5,0xf7,0xd4,0x66,0xa7,0x5a,0x44,0x8a,
          0xc9,0xe8,0xc8,0xe7,0x9a,0x6a,0x73,0x4a,0x61,0xc7,0xf4,0xc6,
          0x65,0xe9,0x72,0xe6,0x71,0x91,0x93,0xa6,0xda,0x92,0x85,0x62,
          0xf3,0xc5,0xb2,0xa4,0x84,0xba,0x64,0xa5,0xb3,0xd2,0x81,0xe5,
          0xd3,0xaa,0xc4,0xca,0xf2,0xb1,0xe4,0xd1,0x83,0x63,0xea,0xc3,
          0xe2,0x82,0xf1,0xa3,0xc2,0xa1,0xc1,0xe3,0xa2,0xe1,0xff,0xff  }
    };

    if (table_idx > 2)
        table_idx = 2;
    memset( m_first_decode, 0, sizeof(m_first_decode));
    memset(m_second_decode, 0, sizeof(m_second_decode));
    make_decoder(m_first_decode,  first_tree[table_idx], 0);
    make_decoder(m_second_decode, second_tree[table_idx], 0);
}

/*
  getbits(-1) initializes the buffer
  getbits(n) where 0 <= n <= 25 returns an n-bit integer
*/
uint32_t CrwDecompressor::getbits(IO::Stream * s, int nbits)
{
    uint32_t ret = 0;
    uint8_t c;
		
    if (nbits == 0) 
        return 0;
    if (nbits == -1)
        ret = m_bitbuf = m_vbits = 0;
    else {
        ret = m_bitbuf << (32 - m_vbits) >> (32 - nbits);
        m_vbits -= nbits;
    }
    while (m_vbits < 25) {
        try {
            c = s->readByte();
            m_bitbuf = (m_bitbuf << 8) + c;
            if (c == 0xff) 
                s->readByte();	/* always extra 00 after ff */
            m_vbits += 8;
        }
        catch(const Internals::IOException &)
        {
            break;
        }
    }
    return ret;
}

namespace {

static
int canon_has_lowbits(IO::Stream * s)
{
    uint8_t test[0x4000 - 26];
    int ret=1;
    uint32_t i;
		
    s->seek (0, SEEK_SET);
    s->read (test, sizeof(test));
    for (i=514; i < sizeof(test) - 1; i++)
        if (test[i] == 0xff) {
            if (test[i+1]) 
                return 1;
            ret=0;
        }
    return ret;
}

}


//	int oldmain(int argc, char **argv)
RawDataPtr CrwDecompressor::decompress()
{
    decode_t *decode, *dindex;
    int i, j, leaf, len, diff, diffbuf[64], r, save;
    int carry = 0, base[2] = {0, 0};
    uint32_t  column = 0;
    uint16_t outbuf[64];
    uint8_t c;

    RawDataPtr bitmap(new RawData);

    bitmap->setDataType(OR_DATA_TYPE_RAW);
    // we know the 10-bits are hardcoded in the CRW
    bitmap->setBpc(10);
    bitmap->setWhiteLevel((1 << 10) - 1);
    uint8_t *rawbuf = (uint8_t*)bitmap->allocData(m_width
                                                  * sizeof(uint16_t)
                                                  * m_height);
    bitmap->setDimensions(m_width,
                          m_height);

    init_tables(m_table);

    int lowbits = canon_has_lowbits(m_stream);
    LOGDBG2("lowbits = %d height = %d width = %d\n", lowbits,
            m_height, m_width);
    m_stream->seek(514 + lowbits * m_height * m_width / 4, SEEK_SET);
    getbits(m_stream, -1);			/* Prime the bit buffer */

    while (column < m_width * m_height) {
        memset(diffbuf, 0, sizeof(diffbuf));
        decode = m_first_decode;
        for (i = 0; i < 64; i++ ) {

            for (dindex = decode; dindex->branch[0]; ) {
                dindex = dindex->branch[getbits(m_stream, 1)];
            }
            leaf = dindex->leaf;
            decode = m_second_decode;

            if (leaf == 0 && i) {
                break;
            }
            if (leaf == 0xff) {
                continue;
            }
            i  += leaf >> 4;
            len = leaf & 15;
            if (len == 0) {
                continue;
            }
            diff = getbits(m_stream, len);
            if ((diff & (1 << (len-1))) == 0) {
                diff -= (1 << len) - 1;
            }
            if (i < 64) {
                diffbuf[i] = diff;
            }
        }
        diffbuf[0] += carry;
        carry = diffbuf[0];
        for (i=0; i < 64; i++ ) {
            if (column++ % m_width == 0) {
                base[0] = base[1] = 512;
            }
            outbuf[i] = (base[i & 1] += diffbuf[i]);
        }
        if (lowbits) {
            save = m_stream->seek(0, SEEK_CUR);
            m_stream->seek((column-64)/4, SEEK_SET);
            for (i=j=0; j < 64/4; j++ ) {
                c = m_stream->readByte();
                for (r = 0; r < 8; r += 2) {
                    // outbuf is 64, so we must check for it to not
                    // overflow (read out of bounds)
                    uint16_t next = i < 63 ? outbuf[i+1] : 0;
                    outbuf[i] = (next << 2) + ((c >> r) & 3);
                    i++;
                }
            }
            m_stream->seek(save, SEEK_SET);
        }
        memcpy(rawbuf, outbuf, 2 * 64);
        rawbuf += 2 * 64;
    }
    return bitmap;
}



} }

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
