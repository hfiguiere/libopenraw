/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ciff/recordentry.cpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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

#include <stdint.h>
#include "ifd.hpp"
#include "ciff/recordentry.hpp"
#include "ciffcontainer.hpp"

namespace OpenRaw {
namespace Internals {
namespace CIFF {

RecordEntry::InHeap::InHeap()
    : length(0), offset(0)
{
}

RecordEntry::InHeap::InHeap(uint32_t _length, uint32_t _offset)
    : length(_length), offset(_offset)
{
}

RecordEntry::RecordEntry()
    : typeCode(0)
{
}

bool RecordEntry::readFrom(const CIFFContainer* container)
{
    auto file = container->file();
    auto endian = container->endian();
    auto result_16 = container->readUInt16(file, endian);
    if (result_16.empty()) {
        return false;
    }
    typeCode = result_16.value();
    if (inRecord()) {
        InRec inrecord;
        int pos = file->seek(0, SEEK_CUR);
        container->fetchData(inrecord.c_array(), pos, 8);
        data = inrecord;
    } else {
        auto result_32 = container->readUInt32(file, endian);
        if (result_32.empty()) {
            return false;
        }
        auto length = result_32.value();
        result_32 = container->readUInt32(file, endian);
        if (result_32.empty()) {
            return false;
        }
        auto offset = result_32.value();
        data = InHeap(length, offset);
    }
    return true;
}

size_t RecordEntry::fetchData(Heap* heap, void* buf, size_t size) const
{
    return heap->container()->fetchData(buf,
                                        offset() + heap->offset(), size);
}

Heap RecordEntry::heap(Heap& h, const CIFFContainer* container) const
{
    return Heap(offset() + h.offset(), length(), container);
}

uint32_t RecordEntry::count() const
{
    auto length = this->length();
    switch (type()) {
    case TYPE_BYTE:
        return length;
    case TYPE_ASCII:
        return length;
    case TYPE_WORD:
        return length / 2;
    case TYPE_DWORD:
        return length / 4;
    default:
        return length;
    }
}

uint16_t RecordEntry::exifType() const
{
    switch (type()) {
    case TYPE_BYTE:
        return IFD::EXIF_FORMAT_BYTE;
    case TYPE_ASCII:
        return IFD::EXIF_FORMAT_ASCII;
    case TYPE_WORD:
        return IFD::EXIF_FORMAT_SHORT;
    case TYPE_DWORD:
        return IFD::EXIF_FORMAT_LONG;
    default:
        return IFD::EXIF_FORMAT_INVALID;
    }
}

uint32_t RecordEntry::containerOffset(const Heap& heap) const
{
    return heap.offset() + offset();
}

std::string RecordEntry::getString(Heap& heap) const
{
    LOGASSERT(type() == TYPE_ASCII);
    char buf[256];
    size_t sz = this->length();
    if(sz > 256) {
        sz = 256;
    }
    /*size_t sz2 = */fetchData(&heap, (void*)buf, sz);
    return buf;
}

}
}
}
