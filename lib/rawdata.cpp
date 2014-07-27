/*
 * libopenraw - rawdata.cpp
 *
 * Copyright (C) 2007, 2012 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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

#include <assert.h>
#include <string.h>

#include <libopenraw++/rawdata.h>
#include <libopenraw++/cfapattern.h>
#include <libopenraw++/rawfile.h>

#include "render/bimedian_demosaic.h"
#include "render/grayscale.h"
#include "trace.h"

namespace OpenRaw {

class RawData::Private {
public:
    RawData *self;
    uint16_t min, max;
    ExifPhotometricInterpretation photometricInterpretation;
    const CfaPattern* cfa_pattern; // IMMUTABLE
    uint32_t compression;
    uint8_t *pos;
    size_t offset;
    size_t row_offset;
    uint8_t slice; /**< the slice index */
    uint32_t sliceWidth; /**< the width of the current slice */
    uint32_t sliceOffset;/**< the offset */

    std::vector<uint16_t> slices; /** all the slice width. */

    double colourMatrix[12];
    uint32_t colourMatrixCount;

    Private(RawData *_self)
        : self(_self), 
          min(0), max(0),
          photometricInterpretation(EV_PI_CFA),
          cfa_pattern(CfaPattern::twoByTwoPattern(OR_CFA_PATTERN_NONE)),
          compression(0),
          pos(NULL), offset(0),
          row_offset(0),
          slice(0), sliceWidth(0),
          sliceOffset(0), slices(),
          colourMatrixCount(0)
        {
            memset(colourMatrix, 0, sizeof(colourMatrix));
        }
    void advance(size_t s);
    void nextSlice();
    void nextRow();
private:
    Private(const Private &);
    Private & operator=(const Private &);
};


RawData *
RawData::getAndExtractRawData(const char* filename, uint32_t options,
                              or_error & err)
{
    err = OR_ERROR_NONE;
    RawData *rawdata = NULL;

    RawFile *file = RawFile::newRawFile(filename);
    if (file) {
        rawdata = new RawData();
        err = file->getRawData(*rawdata, options);
        delete file;
    }
    else {
        err = OR_ERROR_CANT_OPEN; // file error
    }
    return rawdata;
}


RawData::RawData()
    : BitmapData(),
      d(new RawData::Private(this))
{

}


RawData::~RawData()
{
    delete d;
}

// rendering

::or_error RawData::getRenderedImage(BitmapData & bitmapdata, uint32_t /*options*/)
{
	uint32_t _x, _y, out_x, out_y;
	uint16_t *src;

	if(dataType() != OR_DATA_TYPE_RAW) {
		Debug::Trace(DEBUG1) << "wrong data type\n";
		return OR_ERROR_INVALID_FORMAT;
	}
        if(d->photometricInterpretation != EV_PI_CFA &&
           d->photometricInterpretation != EV_PI_LINEAR_RAW) {
            Debug::Trace(DEBUG1) << "only CFA or LinearRaw are supported.\n";
            return OR_ERROR_INVALID_FORMAT;
        }

	or_cfa_pattern pattern;
	pattern = cfaPattern()->patternType();
	_x = width();
	_y = height();
	
	/*
	 rawdata.linearize();
	 rawdata.subtractBlack();
	 rawdata.rescale();
	 rawdata.clip();
	 */
	src = (uint16_t*)data();

        or_error err = OR_ERROR_NONE;

        if (d->photometricInterpretation == EV_PI_CFA) {
            /* figure out how the demosaic can be plugged for a different 
             * algorithm */
            bitmapdata.setDataType(OR_DATA_TYPE_PIXMAP_8RGB);
            uint8_t *dst = (uint8_t *)bitmapdata.allocData(sizeof(uint8_t) * 3 * _x * _y);
            err = bimedian_demosaic(src, _x, _y, pattern, dst, out_x, out_y);
            bitmapdata.setDimensions(out_x, out_y);

            // correct colour using the colour matrices
            // TODO
        }
        else {
            bitmapdata.setDataType(OR_DATA_TYPE_PIXMAP_16RGB);
            uint16_t *dst = (uint16_t *)bitmapdata.allocData(sizeof(uint16_t) 
                                                             * 3 * _x * _y);

            err = grayscale_to_rgb(src, _x, _y, dst);
            bitmapdata.setDimensions(_x, _y);
        }

	return err;
}
	
// other
	
uint16_t RawData::min()
{
    return d->min;
}

uint16_t RawData::max()
{
    return d->max;
}

void RawData::setMin(uint16_t m)
{
    d->min = m;
}

void RawData::setMax(uint16_t m)
{
    d->max = m;
}

void RawData::setPhotometricInterpretation(ExifPhotometricInterpretation pi)
{
    d->photometricInterpretation = pi;
}

ExifPhotometricInterpretation RawData::getPhotometricInterpretation() const
{
    return d->photometricInterpretation;
}


const double* RawData::getColourMatrix1(uint32_t & matrixSize) const
{
    matrixSize = d->colourMatrixCount;
    return d->colourMatrix;
}

void RawData::setColourMatrix1(const double* matrix, uint32_t matrixSize)
{
    if(matrixSize > 12) {
        matrixSize = 12;
    }
    for(uint32_t i = 0; i < 12; i++) {
        d->colourMatrix[i] = matrix[i];
    }
    d->colourMatrixCount = matrixSize;
}

void RawData::swap(RawData & with)
{
    BitmapData::swap(with);
    std::swap(this->d, with.d);
}

void * RawData::allocData(const size_t s)
{
    void * p = BitmapData::allocData(s);
    d->pos = (uint8_t*)p;
    d->offset = 0;
    return p;
}


void RawData::setDimensions(uint32_t _x, uint32_t _y)
{
    BitmapData::setDimensions(_x, _y);
    if(d->slices.size()) {
        d->sliceWidth = d->slices[0];
    }
    else {
        d->sliceWidth = _x;
    }
}

void RawData::setSlices(const std::vector<uint16_t> & slices)
{
    d->slices = slices;
    if(slices.size()) {
        d->sliceWidth = slices[0];
    }
    else {
        d->sliceWidth = width();
    }
}

void RawData::setCfaPatternType(or_cfa_pattern t)
{
    d->cfa_pattern = CfaPattern::twoByTwoPattern(t);
}

const CfaPattern* RawData::cfaPattern() const
{
    return d->cfa_pattern;
}

void RawData::setCfaPattern(const CfaPattern* pattern)
{
    d->cfa_pattern = pattern;
}

void RawData::setCompression(uint32_t t)
{
    d->compression = t;
}

uint32_t RawData::compression() const
{
    return d->compression;
}

#if 0
RawData &RawData::append(uint8_t c)
{
    assert(d->pos);
    assert(d->offset < d->data_size);
    *(d->pos) = c;
    advance(sizeof(c));
    return *this;
}
#endif

RawData &RawData::append(uint16_t c)
{
    assert(d->pos);
    assert(d->offset < size());
    *(d->pos) = c & 0xff;
    *(d->pos + 1) = (c >> 8) & 0xff; 
    d->advance(sizeof(c));
    return *this;
}
	

void RawData::nextRow()
{
    d->nextRow();
}


void RawData::Private::nextRow()
{
    uint32_t w = self->width() * 2;
    uint32_t row = offset / w;
    row++;
    if(row == self->height()) 
    {
        // on the last
        nextSlice();
        row = 0;
    }
    offset = row * w + sliceOffset * 2;
    pos = (uint8_t*)(self->data()) + offset;
    row_offset = offset;
}

void RawData::Private::nextSlice()
{
    if(slices.size() > slice) {
        sliceOffset += slices[slice];
        slice++;
    }
    if(slices.size() > slice) {
        sliceWidth = slices[slice];
    }
    else {
        sliceWidth = 0;
    }
}
	
void RawData::Private::advance(size_t s)
{
    if(offset + s - row_offset >= sliceWidth * 2) {
        nextRow();
    }
    else { 
        pos += s;
        offset += s;
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

