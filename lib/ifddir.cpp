/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ifddir.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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
    auto result = getValue<uint32_t>(IFD::EXIF_TAG_NEW_SUBFILE_TYPE);
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
    LOGDBG1("IfdDir::load() m_offset =%ld\n", m_offset);

    auto file = m_container.file();
    m_entries.clear();
    file->seek(m_offset, SEEK_SET);

    int16_t numEntries = m_container.readInt16(file).unwrap_or(0);
    LOGDBG1("num entries %d\n", numEntries);

    for (int16_t i = 0; i < numEntries; i++) {
        uint32_t data;
        auto id = m_container.readUInt16(file);
        auto type = m_container.readInt16(file);
        auto count = m_container.readInt32(file);
        size_t sz = file->read(&data, 4);
        if (id.empty() || type.empty() || count.empty() || sz != 4) {
          LOGERR("Failed to read entry %d\n", i);
          return false;
        }
        uint16_t n_id = id.unwrap();
        IfdEntry::Ref entry =
          std::make_shared<IfdEntry>(n_id, type.unwrap(),
                                     count.unwrap(), data, m_container);
        m_entries[n_id] = entry;
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
    int16_t numEntries = 0;
    auto file = m_container.file();

    if (m_entries.size() == 0) {
        file->seek(m_offset, SEEK_SET);
        numEntries = m_container.readInt16(file).unwrap_or(0);
        LOGDBG1("numEntries =%d shifting %d bytes\n", numEntries, (numEntries * 12) + 2);
    } else {
        numEntries = m_entries.size();
    }

    file->seek(m_offset + (numEntries * 12) + 2, SEEK_SET);
    // XXX how about we check the error. Even though 0 is not valid.
    return m_container.readInt32(file).unwrap_or(0);
}

/** The SubIFD is locate at offset found in the field
 * EXIF_TAG_SUB_IFDS
 */
IfdDir::Ref IfdDir::getSubIFD(uint32_t idx) const
{
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);

    if (e != nullptr) {
        auto result = e->getArray<uint32_t>();
        if (result.ok()) {
            std::vector<uint32_t> offsets = result.unwrap();
            if (idx >= offsets.size()) {
                Ref ref = std::make_shared<IfdDir>(offsets[idx], m_container);
                ref->load();
                return ref;
            }
        } else {
            LOGERR("Can't get SubIFD offsets\n");
        }
    }
    return Ref();
}

Option<std::vector<IfdDir::Ref>> IfdDir::getSubIFDs()
{
    std::vector<IfdDir::Ref> ifds;
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);
    if (e != nullptr) {
        auto result = e->getArray<uint32_t>();
        if (result.ok()) {
            std::vector<uint32_t> offsets = result.unwrap();
            for (auto offset : offsets) {
                Ref ifd = std::make_shared<IfdDir>(offset, m_container);
                ifd->load();
                ifds.push_back(ifd);
            }
            return Option<std::vector<IfdDir::Ref>>(std::move(ifds));
        }
    }
    return Option<std::vector<IfdDir::Ref>>();
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

    Ref ref = std::make_shared<IfdDir>(val_offset, m_container);
    ref->load();
    return ref;
}

IfdDir::Ref IfdDir::getMakerNoteIfd()
{
    uint32_t val_offset = 0;
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_MAKER_NOTE);
    if (!e) {
        LOGDBG1("MakerNote IFD offset not found.\n");
        return MakerNoteDir::Ref();
    }
    val_offset = e->offset();
    LOGDBG1("MakerNote IFD offset (uncorrected) = %u\n", val_offset);
    val_offset += m_container.exifOffsetCorrection();
    LOGDBG1("MakerNote IFD offset = %u\n", val_offset);

    auto ref = MakerNoteDir::createMakerNote(val_offset, m_container);
    if (ref) {
        ref->load();
    }

    return ref;
}

}
}
