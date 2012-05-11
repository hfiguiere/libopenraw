/*
 * libopenraw - neffile.h
 *
 * Copyright (C) 2006-2008, 2012 Hubert Figuiere
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

#ifndef __NEFFILE_H_
#define __NEFFILE_H_

#include "tiffepfile.h"
#include "huffman.h"
#include <vector>

namespace OpenRaw {

class Thumbnail;

namespace Internals {
class IOFile;
class IfdFileContainer;

class NefFile
    : public TiffEpFile
{
public:
    static RawFile *factory(IO::Stream* _f);
    NefFile(IO::Stream * _f);
    virtual ~NefFile();
    
    /** hack because some (lot?) D100 do set as compressed even though 
     *  it is not
     */
    static bool isCompressed(RawContainer & container, uint32_t offset);
    
    class NEFCompressionInfo {
    public:
        uint16_t vpred[2][2];
        std::vector<uint16_t> curve;
        const HuffmanNode* huffman;
    };
    
protected:
    
    virtual MakerNoteDir::Ref  _locateMakerNoteIfd();

    virtual ::or_error _getColourMatrix(uint32_t index, double* matrix, uint32_t & size);

private:
    
    NefFile(const NefFile&);
    NefFile & operator=(const NefFile &);
    
    static const IfdFile::camera_ids_t s_def[];
    int _getCompressionCurve(RawData&, NEFCompressionInfo&);
    ::or_error _decompressNikonQuantized(RawData&);
    virtual ::or_error _decompressIfNeeded(RawData&, uint32_t);
};
	
}
}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
