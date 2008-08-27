/*
 * libopenraw - rawdata.h
 *
 * Copyright (C) 2007-2008 Hubert Figuiere
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


#ifndef __OPENRAW_RAWDATA_H__
#define __OPENRAW_RAWDATA_H__

#include <vector>

#include <libopenraw/libopenraw.h>
#include <libopenraw++/bitmapdata.h>


namespace OpenRaw {

class RawData
    : public BitmapData
{
public:
    typedef or_cfa_pattern CfaPattern;
    static RawData * getAndExtractRawData(const char* filename, 
                                          uint32_t options,
                                          or_error & err);

    RawData();
    virtual ~RawData();

    uint16_t min();
    uint16_t max();
    void setMin(uint16_t _m);
    void setMax(uint16_t _m);		

    /** swap the two objects data. */
    void swap(RawData & with);

    virtual void *allocData(const size_t s);
    virtual void setDimensions(uint32_t x, uint32_t y);
    void setCfaPattern(CfaPattern t);
    CfaPattern cfaPattern();
    uint32_t compression();
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

    /** private copy constructor to make sure it is not called */
    RawData(const RawData& f);
    /** private = operator to make sure it is never called */
    RawData & operator=(const RawData&);
};

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
