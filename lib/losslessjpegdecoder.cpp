/*******************************************************************************
 * 
 * jrawio - a Java(TM) ImageIO API Spi Provider for RAW files
 * ----------------------------------------------------------
 *
 * Copyright (C) 2003-2006 by Fabrizio Giudici (Fabrizio.Giudici@tidalwave.it)
 * Project home page: http://jrawio.dev.java.net
 * 
 *******************************************************************************
 * 
 * MIT License notice
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 *******************************************************************************
 * 
 * $Id: LosslessJPEGDecoder.java,v 1.2 2006/02/08 19:50:51 fabriziogiudici Exp $
 *  
 ******************************************************************************/


#include <exception>

#include <boost/scoped_array.hpp>
#include <boost/format.hpp>

#include "io/stream.h"
#include "exception.h"
#include "rawcontainer.h"

#include "losslessjpegdecoder.h"




namespace OpenRaw {
	namespace Internals {

		LosslessJPEGDecoder::LosslessJPEGDecoder()
			: bitsPerSample(0),
				m_height(0),
				m_width(0),
				m_channelCount(0),
				m_rowSize(0)
		{
		}

    /*******************************************************************************
     * 
     * Links the decoder to a given input stream. This method also parses the 
     * Lossless JPEG header.
     * 
     * @param   iis          the input stream
     * @throws  IOException  if an I/O error occurs
     * 
     *******************************************************************************/
		void LosslessJPEGDecoder::reset(RawContainer & iis, IO::Stream *s) throw()
		{
			IO::Stream *s = iis.file();
		}
		
    /*******************************************************************************
     * 
     * Loads and decodes another row of data from this encoder.
     * 
     * @param  iis           the bit reader to read data from
     * @throws IOException  if an I/O error occurs
     * 
     *******************************************************************************/

	}
}
