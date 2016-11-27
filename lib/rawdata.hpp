/*
 * libopenraw - rawdata.h
 *
 * Copyright (C) 2007-2008, 2012 Hubert Figuière
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


#ifndef LIBOPENRAWPP_RAWDATA_H_
#define LIBOPENRAWPP_RAWDATA_H_

#include <memory>
#include <vector>

#include <libopenraw/metadata.h>
#include "bitmapdata.hpp"
#include "cfapattern.hpp"

namespace OpenRaw {

class RawData
    : public BitmapData
{
public:
    static RawData * getAndExtractRawData(const char* filename,
                                          uint32_t options,
                                          or_error & err);

    RawData();
    virtual ~RawData();

    RawData(const RawData& f) = delete;
    RawData & operator=(const RawData&) = delete;

    /** Get the rendered image
     * @param bitmapdata the BitmapData to put the image into
     * @param options the option bits. Pass 0 for now.
     * @return the error code
     */
    ::or_error getRenderedImage(BitmapData & bitmapdata, uint32_t options);

    // deprecate rename black level and white level resp.
    uint16_t blackLevel() const;
    uint16_t whiteLevel() const;
    void setBlackLevel(uint16_t _m);
    void setWhiteLevel(uint16_t _m);


    /**
     */
    void setPhotometricInterpretation(ExifPhotometricInterpretation pi);
    ExifPhotometricInterpretation getPhotometricInterpretation() const;

    /** Get colour matrix 1
     * @param index The matrix index.
     * @param size the size of the buffer.
     * @return an array of %size double.
     */
    const double* getColourMatrix1(uint32_t & size) const;
    void setColourMatrix1(const double* matrix, uint32_t size);

    /** Get colour matrix 2
     * @param index The matrix index.
     * @param size the size of the buffer.
     * @return an array of %size double.
     */
    const double* getColourMatrix2(uint32_t & size) const;
    void setColourMatrix2(const double* matrix, uint32_t size);

    /** swap the two objects data. */
    void swap(RawData & with);

    virtual void *allocData(const size_t s) override;
    virtual void setDimensions(uint32_t x, uint32_t y) override;

    void setCfaPatternType(::or_cfa_pattern t);
    /**
     * @return the const CfaPattern*.
     */
    const CfaPattern* cfaPattern() const;
    void setCfaPattern(const CfaPattern* pattern);

    uint32_t compression() const;
    void setCompression(uint32_t c);


    void setSlices(const std::vector<uint16_t> & slices);

    /** append a uint8_t at the current position */
//		BitmapData &append(uint8_t c);
    /** append a uint18_t at the current position */
    RawData &append(uint16_t c);
    /** Jump to next row. Take slicing into account. */
    void nextRow();
private:
    class Private;
    RawData::Private *d;
};


typedef std::unique_ptr<RawData> RawDataPtr;

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
#endif
