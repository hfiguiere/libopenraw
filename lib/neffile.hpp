/* -*- Mode: C++ -*- */
/*
 * libopenraw - neffile.hpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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

#pragma once

#include <stdint.h>

#include <array>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "tiffepfile.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"


namespace OpenRaw {

class RawData;

namespace Internals {

class RawContainer;
struct HuffmanNode;

class NefFile
    : public TiffEpFile
{
    template<typename T>
    friend void audit_coefficients();

public:
    static RawFile *factory(const IO::Stream::Ptr & _f);
    NefFile(const IO::Stream::Ptr & _f);
    virtual ~NefFile();

    NefFile(const NefFile&) = delete;
    NefFile & operator=(const NefFile &) = delete;

    /** hack because some (lot?) D100 do set as compressed even though
     *  it is not
     */
    static bool isCompressed(RawContainer & container, uint32_t offset);
    bool isNrw();

    class NEFCompressionInfo {
    public:
        NEFCompressionInfo()
            : curve(0x8000, 0) {
        }
        uint16_t vpred[2][2];
        std::vector<uint16_t> curve;
        const HuffmanNode* huffman;
    };

protected:

    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;
    virtual uint32_t _translateCompressionType(IFD::TiffCompress tiffCompression) override;
    virtual ::or_error _unpackData(uint16_t bpc, uint32_t compression,
                                   RawData &data, uint32_t x, uint32_t y,
                                   uint32_t offset, uint32_t byte_length) override;

private:
    static const IfdFile::camera_ids_t s_def[];

    ::or_error addThumbnail(std::vector<uint32_t>& list, uint32_t offset, uint32_t len);
    bool _getCompressionCurve(RawData&, NEFCompressionInfo&);
    ::or_error _decompressNikonQuantized(RawData&);
    virtual ::or_error _decompressIfNeeded(RawData&, uint32_t) override;
};

}
}
