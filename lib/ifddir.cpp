/*
 * libopenraw - ifddir.cpp
 *
 * Copyright (C) 2006-2016 Hubert Figuiere
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

#include <fcntl.h>
#include <cstdint>
#include <utility>

#include "trace.hpp"
#include "io/stream.hpp"
#include "ifdfilecontainer.hpp"
#include "ifddir.hpp"
#include "makernotedir.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

bool IfdDir::isPrimary() const
{
    auto result =  getValue<uint32_t>(IFD::EXIF_TAG_NEW_SUBFILE_TYPE);
    return result.ok() && (result.unwrap() == 0);
}

bool IfdDir::isThumbnail() const
{
    auto result = getValue<uint32_t>(IFD::EXIF_TAG_NEW_SUBFILE_TYPE);
    return result.ok() && (result.unwrap() == 1);
}

IfdDir::IfdDir(off_t _offset, IfdFileContainer &_container)
    : m_offset(_offset), m_container(_container), m_entries()
{
}

IfdDir::~IfdDir()
{
}

bool IfdDir::load()
{
    Trace(DEBUG1) << "IfdDir::load() m_offset =" << m_offset << "\n";
    int16_t numEntries = 0;
    auto file = m_container.file();
    m_entries.clear();
    file->seek(m_offset, SEEK_SET);
    m_container.readInt16(file, numEntries);
    Trace(DEBUG1) << "num entries " << numEntries << "\n";
    for (int16_t i = 0; i < numEntries; i++) {
        uint16_t id;
        int16_t type;
        int32_t count;
        uint32_t data;
        m_container.readUInt16(file, id);
        m_container.readInt16(file, type);
        m_container.readInt32(file, count);
        file->read(&data, 4);
        IfdEntry::Ref entry(
            std::make_shared<IfdEntry>(id, type, count, data, m_container));
        m_entries[id] = entry;
    }

    return true;
}

IfdEntry::Ref IfdDir::getEntry(uint16_t id) const
{
    std::map<uint16_t, IfdEntry::Ref>::const_iterator iter;
    iter = m_entries.find(id);
    if (iter != m_entries.end()) {
        return iter->second;
    }
    return IfdEntry::Ref();
}

Option<uint32_t>
IfdDir::getIntegerValue(uint16_t id)
{
    IfdEntry::Ref e = getEntry(id);
    if (e != nullptr) {
        return Option<uint32_t>(e->getIntegerArrayItem(0));
    }
    return Option<uint32_t>();
}

off_t IfdDir::nextIFD()
{
    int16_t numEntries;
    auto file = m_container.file();

    if (m_entries.size() == 0) {
        file->seek(m_offset, SEEK_SET);
        m_container.readInt16(file, numEntries);
        Trace(DEBUG1) << "numEntries =" << numEntries << " shifting "
                      << (numEntries * 12) + 2 << "bytes\n";
    } else {
        numEntries = m_entries.size();
    }

    file->seek(m_offset + (numEntries * 12) + 2, SEEK_SET);
    int32_t next;
    m_container.readInt32(file, next);
    return next;
}

/** The SubIFD is locate at offset found in the field
 * EXIF_TAG_SUB_IFDS
 */
IfdDir::Ref IfdDir::getSubIFD(uint32_t idx) const
{
    std::vector<uint32_t> offsets;
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);
    if (e != nullptr) {
        try {
            e->getArray(offsets);
            if (idx >= offsets.size()) {
                Ref ref(std::make_shared<IfdDir>(offsets[idx], m_container));
                ref->load();
                return ref;
            }
        }
        catch (const std::exception &ex) {
            Trace(ERROR) << "Exception " << ex.what() << "\n";
        }
    }
    return Ref();
}

bool IfdDir::getSubIFDs(std::vector<IfdDir::Ref> &ifds)
{
    bool success = false;
    std::vector<uint32_t> offsets;
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);
    if (e != nullptr) {
        try {
            e->getArray(offsets);
            for (auto iter : offsets) {
                Ref ifd(std::make_shared<IfdDir>(iter, m_container));
                ifd->load();
                ifds.push_back(ifd);
            }
            success = true;
        }
        catch (const std::exception &ex) {
            Trace(ERROR) << "Exception " << ex.what() << "\n";
        }
    }
    return success;
}

/** The SubIFD is located at offset found in the field
 * EXIF_TAG_SUB_IFDS
 */
IfdDir::Ref IfdDir::getExifIFD()
{
    auto result = getValue<uint32_t>(IFD::EXIF_TAG_EXIF_IFD_POINTER);
    if (result.empty()) {
        LOGDBG1("Exif IFD offset not found.\n");
        return Ref();
    }

    uint32_t val_offset = result.unwrap();
    LOGDBG1("Exif IFD offset (uncorrected) = %u\n", val_offset);
    val_offset += m_container.exifOffsetCorrection();
    LOGDBG1("Exif IFD offset = %u\n", val_offset);

    Ref ref(std::make_shared<IfdDir>(val_offset, m_container));
    ref->load();
    return ref;
}

IfdDir::Ref IfdDir::getMakerNoteIfd()
{
    uint32_t val_offset = 0;
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_MAKER_NOTE);
    if (!e) {
        Trace(DEBUG1) << "MakerNote IFD offset not found.\n";
        return MakerNoteDir::Ref();
    }
    val_offset = e->offset();
    Trace(DEBUG1) << "MakerNote IFD offset (uncorrected) = " << val_offset
                  << "\n";
    val_offset += m_container.exifOffsetCorrection();
    Trace(DEBUG1) << "MakerNote IFD offset = " << val_offset << "\n";

    auto ref = MakerNoteDir::createMakerNote(val_offset, m_container);
    if (ref) {
        ref->load();
    }

    return ref;
}

}
}
