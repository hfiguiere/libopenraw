/*
 * libopenraw - ljpegdecompressor.cpp
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
 * Code for JPEG lossless decoding.  Large parts are grabbed from the IJG
 * software, so:
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * Part of the Independent JPEG Group's software.
 * See the file Copyright for more details.
 *
 * Copyright (c) 1993 Brian C. Smith, The Regents of the University
 * of California
 * All rights reserved.
 * 
 * Copyright (c) 1994 Kongji Huang and Brian C. Smith.
 * Cornell University
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF CORNELL
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * CORNELL UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */


#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <algorithm>
#include <cstdint>

#include <boost/format.hpp>

#include <libopenraw/consts.h>
#include <libopenraw/debug.h>

#include "rawdata.hpp"
#include "exception.hpp"
#include "io/stream.hpp"
#include "trace.hpp"
#include "ljpegdecompressor.hpp"
#include "ljpegdecompressor_priv.hpp"

namespace OpenRaw {

using namespace Debug;

namespace Internals {


static void SkipVariable(IO::Stream *s);
static uint16_t Get2bytes (IO::Stream * s);
static int32_t  NextMarker(IO::Stream * );
static void GetSoi(DecompressInfo *dcPtr);
static void GetApp0(IO::Stream *);

LJpegDecompressor::LJpegDecompressor(IO::Stream *stream,
                                     RawContainer *container)
    : Decompressor(stream, container),
      m_slices(),
      m_mcuROW1(NULL), m_mcuROW2(NULL),
      m_buf1(NULL), m_buf2(NULL),
      m_bitsLeft(0),
      m_getBuffer(0)
{
}


LJpegDecompressor::~LJpegDecompressor()
{
    if(m_mcuROW1) {
        free(m_mcuROW1);
    }
    if(m_mcuROW2) {
        free(m_mcuROW2);
    }
    if(m_buf1) {
        free(m_buf1);
    }
    if(m_buf2) {
        free(m_buf2);
    }
}
		

void LJpegDecompressor::setSlices(const std::vector<uint16_t> & slices)
{
    uint16_t n = slices[0];
    m_slices.resize(n + 1);
    for(uint16_t i = 0; i < n; i++) {
        m_slices[i] = slices[1];
    }
    m_slices[n] = slices[2];
}
		


		
static uint32_t bitMask[] = {  0xffffffff, 0x7fffffff, 
                               0x3fffffff, 0x1fffffff,
                               0x0fffffff, 0x07ffffff, 
                               0x03ffffff, 0x01ffffff,
                               0x00ffffff, 0x007fffff, 
                               0x003fffff, 0x001fffff,
                               0x000fffff, 0x0007ffff, 
                               0x0003ffff, 0x0001ffff,
                               0x0000ffff, 0x00007fff, 
                               0x00003fff, 0x00001fff,
                               0x00000fff, 0x000007ff, 
                               0x000003ff, 0x000001ff,
                               0x000000ff, 0x0000007f, 
                               0x0000003f, 0x0000001f,
                               0x0000000f, 0x00000007, 
                               0x00000003, 0x00000001};

void FixHuffTbl (HuffmanTable *htbl);


/*
 *--------------------------------------------------------------
 *
 * FixHuffTbl --
 *
 *      Compute derived values for a Huffman table one the DHT marker
 *      has been processed.  This generates both the encoding and
 *      decoding tables.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
void
FixHuffTbl (HuffmanTable *htbl)
{
    int32_t p, i, l, lastp, si;
    char huffsize[257];
    uint16_t huffcode[257];
    uint16_t code;
    int32_t size;
    int32_t value, ll, ul;
			
    /*
     * Figure C.1: make table of Huffman code length for each symbol
     * Note that this is in code-length order.
     */
    p = 0;
    for (l = 1; l <= 16; l++) {
        for (i = 1; i <= (int)htbl->bits[l]; i++)
            huffsize[p++] = (char)l;
    }
    huffsize[p] = 0;
    lastp = p;
			
			
    /*
     * Figure C.2: generate the codes themselves
     * Note that this is in code-length order.
     */
    code = 0;
    si = huffsize[0];
    p = 0;
    while (huffsize[p]) {
        while (((int)huffsize[p]) == si) {
            huffcode[p++] = code;
            code++;
        }
        code <<= 1;
        si++;
    }
			
    /*
     * Figure C.3: generate encoding tables
     * These are code and size indexed by symbol value
     * Set any codeless symbols to have code length 0; this allows
     * EmitBits to detect any attempt to emit such symbols.
     */
    memset(htbl->ehufsi, 0, sizeof(htbl->ehufsi));
			
    for (p = 0; p < lastp; p++) {
        htbl->ehufco[htbl->huffval[p]] = huffcode[p];
        htbl->ehufsi[htbl->huffval[p]] = huffsize[p];
    }
			
    /*
     * Figure F.15: generate decoding tables
     */
    p = 0;
    for (l = 1; l <= 16; l++) {
        if (htbl->bits[l]) {
            htbl->valptr[l] = p;
            htbl->mincode[l] = huffcode[p];
            p += htbl->bits[l];
            htbl->maxcode[l] = huffcode[p - 1];
        } else {
            htbl->maxcode[l] = -1;
        }
    }
			
    /*
     * We put in this value to ensure HuffDecode terminates.
     */
    htbl->maxcode[17] = 0xFFFFFL;
			
    /*
     * Build the numbits, value lookup tables.
     * These table allow us to gather 8 bits from the bits stream,
     * and immediately lookup the size and value of the huffman codes.
     * If size is zero, it means that more than 8 bits are in the huffman
     * code (this happens about 3-4% of the time).
     */
    bzero (htbl->numbits, sizeof(htbl->numbits));
    for (p=0; p<lastp; p++) {
        size = huffsize[p];
        if (size <= 8) {
            value = htbl->huffval[p];
            code = huffcode[p];
            ll = code << (8-size);
            if (size < 8) {
                ul = ll | bitMask[24+size];
            } else {
                ul = ll;
            }
            for (i=ll; i<=ul; i++) {
                htbl->numbits[i] = size;
                htbl->value[i] = value;
            }
        }
    }
}


#define RST0    0xD0	/* RST0 marker code */


#if 0
/*
 * The following variables keep track of the input buffer
 * for the JPEG data, which is read by ReadJpegData.
 */
uint8_t inputBuffer[JPEG_BUF_SIZE]; /* Input buffer for JPEG data */
int numInputBytes;		/* The total number of bytes in inputBuffer */
int maxInputBytes;		/* Size of inputBuffer */
int inputBufferOffset;		/* Offset of current byte */
#endif


/*
 * Code for extracting the next N bits from the input stream.
 * (N never exceeds 15 for JPEG data.)
 * This needs to go as fast as possible!
 *
 * We read source bytes into getBuffer and dole out bits as needed.
 * If getBuffer already contains enough bits, they are fetched in-line
 * by the macros get_bits() and get_bit().  When there aren't enough bits,
 * fillBitBuffer is called; it will attempt to fill getBuffer to the
 * "high water mark", then extract the desired number of bits.  The idea,
 * of course, is to minimize the function-call overhead cost of entering
 * fillBitBuffer.
 * On most machines MIN_GET_BITS should be 25 to allow the full 32-bit width
 * of getBuffer to be used.  (On machines with wider words, an even larger
 * buffer could be used.)  
 */

#define BITS_PER_LONG	(8*sizeof(int32_t))
#define MIN_GET_BITS  (BITS_PER_LONG-7)	   /* max value for long getBuffer */
		
/*
 * bmask[n] is mask for n rightmost bits
 */
static int32_t bmask[] = {0x0000,
                          0x0001, 0x0003, 0x0007, 0x000F,
                          0x001F, 0x003F, 0x007F, 0x00FF,
                          0x01FF, 0x03FF, 0x07FF, 0x0FFF,
                          0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF};
		

/*
 * Lossless JPEG specifies data precision to be from 2 to 16 bits/sample.
 */ 
#define MinPrecisionBits 2
#define MaxPrecisionBits 16


/*
 *--------------------------------------------------------------
 *
 * DecoderStructInit --
 *
 *	Initalize the rest of the fields in the decompression
 *	structure.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::DecoderStructInit (DecompressInfo *dcPtr)
    noexcept(false)
{
    int16_t ci,i;
    JpegComponentInfo *compPtr;
    int32_t mcuSize;

    /*
     * Check sampling factor validity.
     */
    for (ci = 0; ci < dcPtr->numComponents; ci++) {
        compPtr = &dcPtr->compInfo[ci];
        if ((compPtr->hSampFactor != 1) || (compPtr->vSampFactor != 1)) {
            throw DecodingException("Error: Downsampling is not supported.\n");
        }
    }

    /*
     * Prepare array describing MCU composition
     */
    if (dcPtr->compsInScan == 1) {
        dcPtr->MCUmembership[0] = 0;
    } else {
        if (dcPtr->compsInScan > 4) {
            throw DecodingException("Too many components for interleaved scan");
        }

        for (ci = 0; ci < dcPtr->compsInScan; ci++) {
            dcPtr->MCUmembership[ci] = ci;
        }
    }

    /*
     * Initialize mucROW1 and mcuROW2 which buffer two rows of
     * pixels for predictor calculation.
     */

    if ((m_mcuROW1 = (MCU *)malloc(dcPtr->imageWidth*sizeof(MCU)))==NULL) {
        throw DecodingException("Not enough memory for mcuROW1\n");
    }
    if ((m_mcuROW2 = (MCU *)malloc(dcPtr->imageWidth*sizeof(MCU)))==NULL) {
        throw DecodingException("Not enough memory for mcuROW2\n");
    }

    mcuSize=dcPtr->compsInScan * sizeof(ComponentType);
    if ((m_buf1 = (char *)malloc(dcPtr->imageWidth*mcuSize))==NULL) {
        throw DecodingException("Not enough memory for buf1\n");
    }
    if ((m_buf2 = (char *)malloc(dcPtr->imageWidth*mcuSize))==NULL) {
        throw DecodingException("Not enough memory for buf2\n");
    }

    for (i=0;i<dcPtr->imageWidth;i++) {
        m_mcuROW1[i]=(MCU)(m_buf1+i*mcuSize);
        m_mcuROW2[i]=(MCU)(m_buf2+i*mcuSize);
    }
}


/*
 *--------------------------------------------------------------
 *
 * fillBitBuffer --
 *
 *	Load up the bit buffer with at least nbits
 *	Process any stuffed bytes at this time.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	The bitwise global variables are updated.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::fillBitBuffer (IO::Stream * s,uint16_t nbits)
{
    uint8_t c, c2;
			
    while (m_bitsLeft < MIN_GET_BITS) {
        c = s->readByte();
				
        /*
         * If it's 0xFF, check and discard stuffed zero byte
         */
        if (c == 0xFF) {
            c2 = s->readByte();
					
            if (c2 != 0) {
						
                /*
                 * Oops, it's actually a marker indicating end of
                 * compressed data.  Better put it back for use later.
                 */
                s->seek(-2, SEEK_CUR);
						
                /*
                 * There should be enough bits still left in the data
                 * segment; if so, just break out of the while loop.
                 */
                if (m_bitsLeft >= nbits)
                    break;
						
                /*
                 * Uh-oh.  Corrupted data: stuff zeroes into the data
                 * stream, since this sometimes occurs when we are on the
                 * last show_bits(8) during decoding of the Huffman
                 * segment.
                 */
                c = 0;
            }
        }
        /*
         * OK, load c into getBuffer
         */
        m_getBuffer = (m_getBuffer << 8) | c;
        m_bitsLeft += 8;
    }
}



inline int32_t LJpegDecompressor::QuickPredict(int32_t col, int16_t curComp,
                                               MCU *curRowBuf, 
                                               MCU *prevRowBuf,
                                               int32_t psv)
{
    int32_t left,upper,diag,leftcol;
    int32_t predictor;

    leftcol=col-1;
    upper=prevRowBuf[col][curComp];
    left=curRowBuf[leftcol][curComp];
    diag=prevRowBuf[leftcol][curComp];
	
    /*
     * All predictor are calculated according to psv.
     */
    switch (psv) {
    case 0:
        predictor = 0;
        break;
    case 1:
        predictor = left;
        break;
    case 2:
        predictor = upper;
        break;
    case 3:
        predictor = diag;
        break;
    case 4:
        predictor = left+upper-diag;
        break;
    case 5:
        predictor = left+((upper-diag)>>1);
        break;
    case 6:
        predictor = upper+((left-diag)>>1);
        break;
    case 7:
        predictor = (left+upper)>>1;
        break;
    default:
        LOGWARN("Warning: Undefined PSV\n");
        predictor = 0;
    }
    return predictor;
}

inline
int32_t LJpegDecompressor::show_bits8(IO::Stream * s)
{
    if (m_bitsLeft < 8) {
        fillBitBuffer(s, 8);
    }
    return (m_getBuffer >> (m_bitsLeft-8)) & 0xff;
}

inline
void LJpegDecompressor::flush_bits(uint16_t nbits)
{
    m_bitsLeft -= (nbits);
}

inline
int32_t LJpegDecompressor::get_bits(uint16_t nbits)
{
    if (m_bitsLeft < nbits) 
        fillBitBuffer(m_stream, nbits);
    return ((m_getBuffer >> (m_bitsLeft -= (nbits)))) & bmask[nbits];
}
		
inline
int32_t LJpegDecompressor::get_bit() 
{
    if (!m_bitsLeft) 
        fillBitBuffer(m_stream, 1);
    return (m_getBuffer >> (--m_bitsLeft)) & 1;
}


inline
int32_t LJpegDecompressor::readBits(IO::Stream * s, uint16_t nbits)
{
    if (m_bitsLeft < nbits) {
        fillBitBuffer(s, nbits);
    }
    return ((m_getBuffer >> (m_bitsLeft -= (nbits)))) & bmask[nbits];
}



/*
 *--------------------------------------------------------------
 *
 * PmPutRow --
 *
 *      Output one row of pixels stored in RowBuf.
 *
 * Results:
 *      None
 *
 * Side effects:
 *      One row of pixels are write to file pointed by outFile.
 *
 *--------------------------------------------------------------
 */
inline void
LJpegDecompressor::PmPutRow(MCU* RowBuf, int32_t numComp, int32_t numCol, int32_t Pt)
{
    // TODO this might be wrong in 8 bits...
    // original code was using putc which *i think* was a problem for
    // 16bpp
    int32_t comp;
    int32_t col;
    uint16_t v;

    for (col = 0; col < numCol; col++) {
        for (comp = 0; comp < numComp; comp++) {
            v = RowBuf[col][comp]<<Pt;
            m_output->append(v);
        }
    }
//			m_output->nextRow();
}

/*
 *--------------------------------------------------------------
 *
 * HuffDecode --
 *
 *	Taken from Figure F.16: extract next coded symbol from
 *	input stream.  This should becode a macro.
 *
 * Results:
 *	Next coded symbol
 *
 * Side effects:
 *	Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
inline int32_t 
LJpegDecompressor::HuffDecode(HuffmanTable *htbl)
{
    int32_t rv;
    int32_t l, temp;
    int32_t code;

    /*
     * If the huffman code is less than 8 bits, we can use the fast
     * table lookup to get its value.  It's more than 8 bits about
     * 3-4% of the time.
     */
    code = show_bits8(m_stream);
    if (htbl->numbits[code]) {
        flush_bits(htbl->numbits[code]);
        rv=htbl->value[code];
    }  else {
        flush_bits(8);
        l = 8;
        while (code > htbl->maxcode[l]) {
            temp = get_bit();
            code = (code << 1) | temp;
            l++;
        }

        /*
         * With garbage input we may reach the sentinel value l = 17.
         */

        if (l > 16) {
            //LOGWARN("Corrupt JPEG data: bad Huffman code %d\n", l);
            rv = 0;		/* fake a zero as the safest result */
        } else {
            rv = htbl->huffval[htbl->valptr[l] +
                               ((int)(code - htbl->mincode[l]))];
        }
    }
    return rv;
}

/*
 *--------------------------------------------------------------
 *
 * HuffExtend --
 *
 *	Code and table for Figure F.12: extend sign bit
 *
 * Results:
 *	The extended value.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
static const int32_t extendTest[16] =	/* entry n is 2**(n-1) */
{0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000};

// we can't bitshift -1. So we use 0xffffffff as a value.
// gcc complain about it otherwise.
#define EXTEND(n) (int32_t)(0xffffffff << n) + 1
static const int32_t extendOffset[16] =	/* entry n is (-1 << n) + 1 */
{
    0, EXTEND(1), EXTEND(2), EXTEND(3),
    EXTEND(4), EXTEND(5), EXTEND(6), EXTEND(7),
    EXTEND(8), EXTEND(9), EXTEND(10), EXTEND(11),
    EXTEND(12), EXTEND(13), EXTEND(14), EXTEND(15)
};

inline
void HuffExtend(int32_t & x, int32_t s) noexcept
{
    if ((x) < extendTest[s]) {
        (x) += extendOffset[s];
    }
}

/*
 *--------------------------------------------------------------
 *
 * HuffDecoderInit --
 *
 *	Initialize for a Huffman-compressed scan.
 *	This is invoked after reading the SOS marker.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::HuffDecoderInit (DecompressInfo *dcPtr)
    noexcept(false)
{
    int16_t ci;
    JpegComponentInfo *compptr;

    /*
     * Initialize static variables
     */
    m_bitsLeft = 0;

    for (ci = 0; ci < dcPtr->compsInScan; ci++) {
        compptr = dcPtr->curCompInfo[ci];
        /*
         * Make sure requested tables are present
         */
        if (dcPtr->dcHuffTblPtrs[compptr->dcTblNo] == NULL) { 
            throw DecodingException("Error: Use of undefined Huffman table\n");
        }

        /*
         * Compute derived values for Huffman tables.
         * We may do this more than once for same table, but it's not a
         * big deal
         */
        FixHuffTbl (dcPtr->dcHuffTblPtrs[compptr->dcTblNo]);
    }

    /*
     * Initialize restart stuff
     */
    dcPtr->restartInRows = (dcPtr->restartInterval)/(dcPtr->imageWidth);
    dcPtr->restartRowsToGo = dcPtr->restartInRows;
    dcPtr->nextRestartNum = 0;
}

/*
 *--------------------------------------------------------------
 *
 * ProcessRestart --
 *
 *	Check for a restart marker & resynchronize decoder.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	BitStream is parsed, bit buffer is reset, etc.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::ProcessRestart (DecompressInfo *dcPtr)
    noexcept(false)
{
    int32_t c, nbytes;

    /*
     * Throw away any unused bits remaining in bit buffer
     */
    nbytes = m_bitsLeft / 8;
    m_bitsLeft = 0;

    /*
     * Scan for next JPEG marker
     */
    do {
        do {			/* skip any non-FF bytes */
            nbytes++;
            c = m_stream->readByte();
        } while (c != 0xFF);
        do {			/* skip any duplicate FFs */
            /*
             * we don't increment nbytes here since extra FFs are legal
             */
            c = m_stream->readByte();
        } while (c == 0xFF);
    } while (c == 0);		/* repeat if it was a stuffed FF/00 */

    if (c != (RST0 + dcPtr->nextRestartNum)) {

        /*
         * Uh-oh, the restart markers have been messed up too.
         * Just bail out.
         */
        throw DecodingException("Error: Corrupt JPEG data. "
                                "Aborting decoding...\n");
    }

    /*
     * Update restart state
     */
    dcPtr->restartRowsToGo = dcPtr->restartInRows;
    dcPtr->nextRestartNum = (dcPtr->nextRestartNum + 1) & 7;
}

/*
 *--------------------------------------------------------------
 *
 * DecodeFirstRow --
 *
 *	Decode the first raster line of samples at the start of 
 *      the scan and at the beginning of each restart interval.
 *	This includes modifying the component value so the real
 *      value, not the difference is returned.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
void LJpegDecompressor::DecodeFirstRow(DecompressInfo *dcPtr,
                                       MCU *curRowBuf)
{
    uint16_t curComp,ci;
    int32_t s,col,compsInScan,numCOL;
    JpegComponentInfo *compptr;
    int32_t Pr,Pt,d;
    HuffmanTable *dctbl;

    Pr=dcPtr->dataPrecision;
    Pt=dcPtr->Pt;
    compsInScan=dcPtr->compsInScan;
    numCOL=dcPtr->imageWidth;

    /*
     * the start of the scan or at the beginning of restart interval.
     */
    for (curComp = 0; curComp < compsInScan; curComp++) {
        ci = dcPtr->MCUmembership[curComp];
        compptr = dcPtr->curCompInfo[ci];
        dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

        /*
         * Section F.2.2.1: decode the difference
         */
        s = HuffDecode (dctbl);
        if (s) {
            d = get_bits(s);
            HuffExtend(d,s);
        } else {
            d = 0;
        }

        /* 
         * Add the predictor to the difference.
         */
        curRowBuf[0][curComp]=d+(1<<(Pr-Pt-1));
    }

    /*
     * the rest of the first row
     */
    for (col=1; col<numCOL; col++) {
        for (curComp = 0; curComp < compsInScan; curComp++) {
            ci = dcPtr->MCUmembership[curComp];
            compptr = dcPtr->curCompInfo[ci];
            dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

            /*
             * Section F.2.2.1: decode the difference
             */
            s = HuffDecode (dctbl);
            if (s) {
                d = get_bits(s);
                HuffExtend(d,s);
            } else {
                d = 0;
            }

            /* 
             * Add the predictor to the difference.
             */
            curRowBuf[col][curComp]=d+curRowBuf[col-1][curComp];
        }
    }

    if (dcPtr->restartInRows) {
        (dcPtr->restartRowsToGo)--;
    }
}

/*
 *--------------------------------------------------------------
 *
 * DecodeImage --
 *
 *      Decode the input stream. This includes modifying
 *      the component value so the real value, not the
 *      difference is returned.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::DecodeImage(DecompressInfo *dcPtr)
{
    int32_t s,d,col,row;
    int16_t curComp, ci;
    HuffmanTable *dctbl;
    JpegComponentInfo *compptr;
    int32_t predictor;
    int32_t numCOL,numROW,compsInScan;
    MCU *prevRowBuf,*curRowBuf;
    int32_t imagewidth,Pt,psv;

    numCOL=imagewidth=dcPtr->imageWidth;
    numROW=dcPtr->imageHeight;
    compsInScan=dcPtr->compsInScan;
    Pt=dcPtr->Pt;
    psv=dcPtr->Ss;
    prevRowBuf=m_mcuROW2;
    curRowBuf=m_mcuROW1;

    /*
     * Decode the first row of image. Output the row and
     * turn this row into a previous row for later predictor
     * calculation.
     */  
    DecodeFirstRow(dcPtr,curRowBuf);
    PmPutRow(curRowBuf,compsInScan,numCOL,Pt);
    std::swap(prevRowBuf,curRowBuf);

    for (row=1; row<numROW; row++) {

        /*
         * Account for restart interval, process restart marker if needed.
         */
        if (dcPtr->restartInRows) {
            if (dcPtr->restartRowsToGo == 0) {
                ProcessRestart (dcPtr);
            
                /*
                 * Reset predictors at restart.
                 */
                DecodeFirstRow(dcPtr,curRowBuf);
                PmPutRow(curRowBuf,compsInScan,numCOL,Pt);
                std::swap(prevRowBuf,curRowBuf);
                continue;
            }
            dcPtr->restartRowsToGo--;
        }

        /*
         * The upper neighbors are predictors for the first column.
         */
        for (curComp = 0; curComp < compsInScan; curComp++) {
            ci = dcPtr->MCUmembership[curComp];
            compptr = dcPtr->curCompInfo[ci];
            dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

            /*
             * Section F.2.2.1: decode the difference
             */
            s = HuffDecode (dctbl);
            if (s) {
                d = get_bits(s);
                HuffExtend(d,s);
            } else {
                d = 0;
            }

            curRowBuf[0][curComp]=d+prevRowBuf[0][curComp];
        }

        /*
         * For the rest of the column on this row, predictor
         * calculations are base on PSV. 
         */
        for (col=1; col<numCOL; col++) {
            for (curComp = 0; curComp < compsInScan; curComp++) {
                ci = dcPtr->MCUmembership[curComp];
                compptr = dcPtr->curCompInfo[ci];
                dctbl = dcPtr->dcHuffTblPtrs[compptr->dcTblNo];

                /*
                 * Section F.2.2.1: decode the difference
                 */
                s = HuffDecode (dctbl);
                if (s) {
                    d = get_bits(s);
                    HuffExtend(d,s);
                } else {
                    d = 0;
                }
                predictor = QuickPredict(col,curComp,curRowBuf,prevRowBuf,
                                         psv);

                curRowBuf[col][curComp]=d+predictor;
            }
        }
        PmPutRow(curRowBuf,compsInScan,numCOL,Pt);
        std::swap(prevRowBuf,curRowBuf);
    }
}




/*
 *--------------------------------------------------------------
 *
 * Get2bytes --
 *
 *	Get a 2-byte unsigned integer (e.g., a marker parameter length
 *	field)
 *
 * Results:
 *	Next two byte of input as an integer.
 *
 * Side effects:
 *	Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
static inline uint16_t
Get2bytes (IO::Stream * s)
{
    uint16_t a;

    a = s->readByte();
    return (a << 8) | s->readByte();
}

/*
 *--------------------------------------------------------------
 *
 * SkipVariable --
 *
 *	Skip over an unknown or uninteresting variable-length marker
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Bitstream is parsed over marker.
 *
 *
 *--------------------------------------------------------------
 */
static inline void SkipVariable(IO::Stream * s)
{
    int32_t length;

    length = Get2bytes(s) - 2;

    s->seek(length, SEEK_CUR);
}

/*
 *--------------------------------------------------------------
 *
 * GetDht --
 *
 *	Process a DHT marker
 *
 * Results:
 *	None
 *
 * Side effects:
 *	A huffman table is read.
 *	Exits on error.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::GetDht (DecompressInfo *dcPtr)
    noexcept(false)
{
    int32_t length;
    int32_t i, index, count;

    length = Get2bytes(m_stream) - 2;

    while (length) {
        index = m_stream->readByte();

        if (index < 0 || index >= 4) {
            throw DecodingException(str(boost::format("Bogus DHT index %1%")
                                        % index));
        }

        HuffmanTable *& htblptr = dcPtr->dcHuffTblPtrs[index];
        if (htblptr == NULL) {
            htblptr = (HuffmanTable *) malloc(sizeof (HuffmanTable));
            if (htblptr==NULL) {
                throw DecodingException("Can't malloc HuffmanTable");
            }
        }

        htblptr->bits[0] = 0;
        count = 0;
        for (i = 1; i <= 16; i++) {
            htblptr->bits[i] = m_stream->readByte();
            count += htblptr->bits[i];
        }

        if (count > 256) {
            throw DecodingException("Bogus DHT counts");
        }

        for (i = 0; i < count; i++)
            htblptr->huffval[i] = m_stream->readByte();

        length -= 1 + 16 + count;
    }
}

/*
 *--------------------------------------------------------------
 *
 * GetDri --
 *
 *	Process a DRI marker
 *
 * Results:
 *	None
 *
 * Side effects:
 *	Exits on error.
 *	Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::GetDri(DecompressInfo *dcPtr)
    noexcept(false)
{
    if (Get2bytes(m_stream) != 4) {
        throw DecodingException("Bogus length in DRI");
    }

    dcPtr->restartInterval = Get2bytes(m_stream);
}

/*
 *--------------------------------------------------------------
 *
 * GetApp0 --
 *
 *	Process an APP0 marker.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	Bitstream is parsed
 *
 *--------------------------------------------------------------
 */
static void	GetApp0(IO::Stream *s)
{
    int32_t length;

    length = Get2bytes(s) - 2;
    s->seek(length, SEEK_CUR);
}

/*
 *--------------------------------------------------------------
 *
 * GetSof --
 *
 *	Process a SOFn marker
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Bitstream is parsed
 *	Exits on error
 *	dcPtr structure is filled in
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::GetSof(DecompressInfo *dcPtr)
    noexcept(false)
{
    int32_t length;
    int16_t ci;
    int32_t c;
    JpegComponentInfo *compptr;

    length = Get2bytes(m_stream);

    dcPtr->dataPrecision = m_stream->readByte();
    dcPtr->imageHeight = Get2bytes(m_stream);
    dcPtr->imageWidth = Get2bytes(m_stream);
    dcPtr->numComponents = m_stream->readByte();

    /*
     * We don't support files in which the image height is initially
     * specified as 0 and is later redefined by DNL.  As long as we
     * have to check that, might as well have a general sanity check.
     */
    if ((dcPtr->imageHeight <= 0 ) ||
        (dcPtr->imageWidth <= 0) || 
        (dcPtr->numComponents <= 0)) {
        throw DecodingException("Empty JPEG image (DNL not supported)");
    }

    if ((dcPtr->dataPrecision<MinPrecisionBits) ||
        (dcPtr->dataPrecision>MaxPrecisionBits)) {
        throw DecodingException("Unsupported JPEG data precision");
    }

    if (length != (dcPtr->numComponents * 3 + 8)) {
        throw DecodingException("Bogus SOF length");
    }

    dcPtr->compInfo = (JpegComponentInfo *) malloc
        (dcPtr->numComponents * sizeof (JpegComponentInfo));

    for (ci = 0; ci < dcPtr->numComponents; ci++) {
        compptr = &dcPtr->compInfo[ci];
        compptr->componentIndex = ci;
        compptr->componentId = m_stream->readByte();
        c = m_stream->readByte();
        compptr->hSampFactor = (int16_t)((c >> 4) & 15);
        compptr->vSampFactor = (int16_t)((c) & 15);
        (void) m_stream->readByte();   /* skip Tq */
    }
}

/*
 *--------------------------------------------------------------
 *
 * GetSos --
 *
 *	Process a SOS marker
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Bitstream is parsed.
 *	Exits on error.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::GetSos (DecompressInfo *dcPtr)
    noexcept(false)
{
    int32_t length;
    int32_t i;
    uint16_t n, ci, c, cc;
    JpegComponentInfo *compptr;

    length = Get2bytes (m_stream);

    /* 
     * Get the number of image components.
     */
    n = m_stream->readByte();
    dcPtr->compsInScan = n;
    length -= 3;

    if (length != (n * 2 + 3) || n < 1 || n > 4) {
        throw DecodingException("Bogus SOS length");
    }


    for (i = 0; i < n; i++) {
        cc = m_stream->readByte();
        c = m_stream->readByte();
        length -= 2;

        for (ci = 0; ci < dcPtr->numComponents; ci++)
            if (cc == dcPtr->compInfo[ci].componentId) {
                break;
            }

        if (ci >= dcPtr->numComponents) {
            throw DecodingException("Invalid component number in SOS");
        }

        compptr = &dcPtr->compInfo[ci];
        dcPtr->curCompInfo[i] = compptr;
        compptr->dcTblNo = (c >> 4) & 15;
    }

    /*
     * Get the PSV, skip Se, and get the point transform parameter.
     */
    dcPtr->Ss = m_stream->readByte(); 
    (void)m_stream->readByte();
    c = m_stream->readByte(); 
    dcPtr->Pt = c & 0x0F;
}

/*
 *--------------------------------------------------------------
 *
 * GetSoi --
 *
 *	Process an SOI marker
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Bitstream is parsed.
 *	Exits on error.
 *
 *--------------------------------------------------------------
 */
static inline void
GetSoi (DecompressInfo *dcPtr)
{

    /*
     * Reset all parameters that are defined to be reset by SOI
     */
    dcPtr->restartInterval = 0;
}

/*
 *--------------------------------------------------------------
 *
 * NextMarker --
 *
 *      Find the next JPEG marker Note that the output might not
 *	be a valid marker code but it will never be 0 or FF
 *
 * Results:
 *	The marker found.
 *
 * Side effects:
 *	Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
static int32_t
NextMarker(IO::Stream *s)
{
    int32_t c;

    do {
        /*
         * skip any non-FF bytes
         */
        do {
            c = s->readByte();
        } while (c != 0xFF);
        /*
         * skip any duplicate FFs without incrementing nbytes, since
         * extra FFs are legal
         */
        do {
            c = s->readByte();
        } while (c == 0xFF);
    } while (c == 0);		/* repeat if it was a stuffed FF/00 */

    return c;
}

/*
 *--------------------------------------------------------------
 *
 * ProcessTables --
 *
 *	Scan and process JPEG markers that can appear in any order
 *	Return when an SOI, EOI, SOFn, or SOS is found
 *
 * Results:
 *	The marker found.
 *
 * Side effects:
 *	Bitstream is parsed.
 *
 *--------------------------------------------------------------
 */
LJpegDecompressor::JpegMarker
LJpegDecompressor::ProcessTables (DecompressInfo *dcPtr)
{
    int c;

    while (1) {
        c = NextMarker (m_stream);

        switch (c) {
        case M_SOF0:
        case M_SOF1:
        case M_SOF2:
        case M_SOF3:
        case M_SOF5:
        case M_SOF6:
        case M_SOF7:
        case M_JPG:
        case M_SOF9:
        case M_SOF10:
        case M_SOF11:
        case M_SOF13:
        case M_SOF14:
        case M_SOF15:
        case M_SOI:
        case M_EOI:
        case M_SOS:
            return ((JpegMarker)c);

        case M_DHT:
            GetDht (dcPtr);
            break;

        case M_DQT:
            LOGWARN("Not a lossless JPEG file.\n");
            break;

        case M_DRI:
            GetDri (dcPtr);
            break;

        case M_APP0:
            GetApp0(m_stream);
            break;

        case M_RST0:		/* these are all parameterless */
        case M_RST1:
        case M_RST2:
        case M_RST3:
        case M_RST4:
        case M_RST5:
        case M_RST6:
        case M_RST7:
        case M_TEM:
            LOGWARN("Warning: unexpected marker 0x%x", c);
            break;

        default:		/* must be DNL, DHP, EXP, APPn, JPGn, COM,
                         * or RESn */
            SkipVariable (m_stream);
            break;
        }
    }
}

/*
 *--------------------------------------------------------------
 *
 * ReadFileHeader --
 *
 *	Initialize and read the file header (everything through
 *	the SOF marker).
 *
 * Results:
 *	None
 *
 * Side effects:
 *	Exit on error.
 *
 *--------------------------------------------------------------
 */
void
LJpegDecompressor::ReadFileHeader (DecompressInfo *dcPtr)
    noexcept(false)
{
    int c, c2;

    /*
     * Demand an SOI marker at the start of the file --- otherwise it's
     * probably not a JPEG file at all.
     */
    c = m_stream->readByte();
    c2 = m_stream->readByte();
    if ((c != 0xFF) || (c2 != M_SOI)) {
        throw DecodingException(str(boost::format("Not a JPEG file. "
                                                  "marker is %1% %2%\n")
                                    % c % c2));
    }

    GetSoi (dcPtr);		/* OK, process SOI */

    /*
     * Process markers until SOF
     */
    c = ProcessTables (dcPtr);

    switch (c) {
    case M_SOF0:
    case M_SOF1:
    case M_SOF3:
        GetSof(dcPtr);
        break;

    default:
        LOGWARN("Unsupported SOF marker type 0x%x\n", c);
        break;
    }
}

/*
 *--------------------------------------------------------------
 *
 * ReadScanHeader --
 *
 *	Read the start of a scan (everything through the SOS marker).
 *
 * Results:
 *	1 if find SOS, 0 if find EOI
 *
 * Side effects:
 *	Bitstream is parsed, may exit on errors.
 *
 *--------------------------------------------------------------
 */
int32_t
LJpegDecompressor::ReadScanHeader (DecompressInfo *dcPtr)
{
    int c;

    /*
     * Process markers until SOS or EOI
     */
    c = ProcessTables (dcPtr);

    switch (c) {
    case M_SOS:
        GetSos (dcPtr);
        return 1;

    case M_EOI:
        return 0;

    default:
        LOGWARN("Unexpected marker 0x%x\n", c);
        break;
    }
    return 0;
}


RawDataPtr LJpegDecompressor::decompress()
{
    DecompressInfo dcInfo;
    try {
        ReadFileHeader(&dcInfo); 
        ReadScanHeader (&dcInfo);

        m_output = RawDataPtr(new RawData);
        m_output->setDataType(OR_DATA_TYPE_RAW);
        uint32_t bpc = dcInfo.dataPrecision;

        m_output->setBpc(bpc);
        m_output->setWhiteLevel((1 << bpc) - 1);
        /*uint16_t *dataPtr = (uint16_t*)*/
        m_output->allocData(dcInfo.imageWidth
                          * sizeof(uint16_t) 
                          * dcInfo.imageHeight
                          * dcInfo.numComponents);

        LOGDBG1("dc width = %d dc height = %d\n", dcInfo.imageWidth,
                dcInfo.imageHeight);
        /* consistently the real width is the JPEG width * numComponent
         * at least with all the Canon.
         * @todo check that this is valid with DNG too.
         */
        uint32_t width = dcInfo.imageWidth * dcInfo.numComponents;
        m_output->setDimensions(width, dcInfo.imageHeight);
        m_output->setSlices(m_slices);
        DecoderStructInit(&dcInfo);
        HuffDecoderInit(&dcInfo);
        DecodeImage(&dcInfo);
        // TODO handle the error properly
    }
    catch(...)
    {
        LOGERR("Decompression error\n");
    }
    return std::move(m_output);
}

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
