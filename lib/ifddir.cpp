/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ifddir.cpp
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

#include <fcntl.h>
#include <cstdint>
#include <utility>

#include "trace.hpp"
#include "io/stream.hpp"
#include "ifdfilecontainer.hpp"
#include "ifddir.hpp"
#include "makernotedir.hpp"
#include "metavalue.hpp"
#include "exif/exif_tags.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

bool IfdDir::isPrimary() const
{
    auto result = getValue<uint32_t>(IFD::EXIF_TAG_NEW_SUBFILE_TYPE);
    return result && (result.value() == 0);
}

bool IfdDir::isThumbnail() const
{
    auto result = getValue<uint32_t>(IFD::EXIF_TAG_NEW_SUBFILE_TYPE);
    return result && (result.value() == 1);
}

IfdDir::IfdDir(off_t _offset, const IfdFileContainer& _container, IfdDirType _type, const TagTable& tag_table)
    : m_type(_type)
    , m_offset(_offset), m_container(_container), m_entries()
    , m_tag_table(&tag_table)
    , m_base_offset(0)
    , m_endian(_container.endian())
{
}

IfdDir::~IfdDir()
{
}

bool IfdDir::load()
{
    LOGDBG1("IfdDir::load() m_offset =%lld\n", (long long int)m_offset);

    auto file = m_container.file();
    m_entries.clear();
    file->seek(m_offset, SEEK_SET);

    int16_t numEntries = m_container.readInt16(file, m_endian).value_or(0);
    LOGDBG1("num entries %d\n", numEntries);

    for (int16_t i = 0; i < numEntries; i++) {
        uint32_t data;
        auto id = m_container.readUInt16(file, m_endian);
        auto type = m_container.readInt16(file, m_endian);
        auto count = m_container.readInt32(file, m_endian);
        size_t sz = file->read(&data, 4);
        if (id.empty() || type.empty() || count.empty() || sz != 4) {
          LOGERR("Failed to read entry %d\n", i);
          return false;
        }
        uint16_t n_id = id.value();
        IfdEntry::Ref entry =
          std::make_shared<IfdEntry>(n_id, type.value(),
                                     count.value(), data, *this);
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
    if (e) {
        return Option<uint32_t>(getEntryIntegerArrayItemValue(*e, 0));
    }
    return Option<uint32_t>();
}

off_t IfdDir::nextIFD()
{
    int16_t numEntries = 0;
    auto file = m_container.file();

    if (m_entries.size() == 0) {
        file->seek(m_offset, SEEK_SET);
        numEntries = m_container.readInt16(file, m_endian).value_or(0);
        LOGDBG1("numEntries =%d shifting %d bytes\n", numEntries, (numEntries * 12) + 2);
    } else {
        numEntries = m_entries.size();
    }

    file->seek(m_offset + (numEntries * 12) + 2, SEEK_SET);
    // XXX how about we check the error. Even though 0 is not valid.
    return m_container.readInt32(file, m_endian).value_or(0);
}

/** The SubIFD is locate at offset found in the field
 * EXIF_TAG_SUB_IFDS
 */
IfdDir::Ref IfdDir::getSubIFD(uint32_t idx) const
{
    IfdEntry::Ref e = getEntry(IFD::EXIF_TAG_SUB_IFDS);

    if (e != nullptr) {
        auto result = getEntryArrayValue<uint32_t>(*e);
        if (result) {
            std::vector<uint32_t> offsets = result.value();
            if (idx >= offsets.size()) {
                Ref ref = std::make_shared<IfdDir>(offsets[idx], m_container, OR_IFD_OTHER);
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
        auto result = getEntryArrayValue<uint32_t>(*e);
        if (result) {
            std::vector<uint32_t> offsets = result.value();
            for (auto offset_ : offsets) {
                Ref ifd = std::make_shared<IfdDir>(offset_, m_container, OR_IFD_OTHER);
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

    uint32_t val_offset = result.value();
    LOGDBG1("Exif IFD offset (uncorrected) = %u\n", val_offset);
    val_offset += m_container.exifOffsetCorrection();
    LOGDBG1("Exif IFD offset = %u\n", val_offset);

    Ref ref = std::make_shared<IfdDir>(val_offset, m_container, OR_IFD_EXIF);
    ref->load();
    return ref;
}

IfdDir::Ref IfdDir::getMakerNoteIfd(or_rawfile_type file_type)
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

    auto ref = MakerNoteDir::createMakerNote(val_offset, m_container, file_type);
    if (ref) {
        ref->load();
    }

    return ref;
}

const char* IfdDir::getTagName(uint32_t tag) const
{
    auto iter = m_tag_table->find(tag);
    if (iter != m_tag_table->end()) {
        return iter->second;
    }
    return nullptr;
}

size_t IfdDir::getEntryData(IfdEntry& e, uint8_t* buffer, size_t buffersize) const
{
    auto loaded = e.loadDataInto(buffer, buffersize, m_base_offset);
    return loaded;
}

uint32_t IfdDir::getEntryIntegerArrayItemValue(IfdEntry& e, int idx) const
{
    uint32_t v = 0;

    try {
        switch (e.type())
        {
        case IFD::EXIF_FORMAT_LONG:
            v = getEntryValue<uint32_t>(e, idx);
            break;
        case IFD::EXIF_FORMAT_SHORT:
            v = getEntryValue<uint16_t>(e, idx);
            break;
        case IFD::EXIF_FORMAT_RATIONAL:
        {
            IFD::ORRational r = getEntryValue<IFD::ORRational>(e, idx);
            if (r.denom == 0) {
                v = 0;
            } else {
                v = r.num / r.denom;
            }
            break;
        }
        default:
            break;
        }
    }
    catch (const std::exception & ex) {
        LOGERR("Exception raised %s fetch integer value for %d\n", ex.what(), e.id());
    }

    return v;
}

namespace {

template<class T>
void convert(const IfdDir& dir, Internals::IfdEntry& e, std::vector<MetaValue::value_t>& values)
{
    auto result = dir.getEntryArrayValue<T>(e);
    LOGASSERT(!!result);
    if (result) {
        std::vector<T> v = result.value();
        values.insert(values.end(), v.cbegin(), v.cend());
    }
}

// T is the Ifd primitive type. T2 is the target MetaValue type.
template<class T, class T2>
void convert(const IfdDir& dir, Internals::IfdEntry& e, std::vector<MetaValue::value_t>& values)
{
    auto result = dir.getEntryArrayValue<T>(e);
    LOGASSERT(!!result);
    if (result) {
        std::vector<T> v = result.value();
        for (const auto & elem : v) {
            values.push_back(T2(elem));
        }
    }
}

}

MetaValue* IfdDir::makeMetaValue(IfdEntry& entry) const
{
    std::vector<MetaValue::value_t> values;

    switch (entry.type()) {
    case Internals::IFD::EXIF_FORMAT_BYTE:
    {
        convert<uint8_t, uint32_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_ASCII:
    {
        convert<std::string>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SHORT:
    {
        convert<uint16_t, uint32_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_LONG:
    {
        convert<uint32_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_RATIONAL:
    {
        convert<Internals::IFD::ORRational>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SBYTE:
    {
        convert<int8_t, int32_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_UNDEFINED:
    {
        convert<uint8_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SSHORT:
    {
        convert<int16_t, int32_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SLONG:
    {
        convert<int32_t>(*this, entry, values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SRATIONAL:
    {
        convert<Internals::IFD::ORSRational>(*this, entry, values);
        break;
    }
    default:
        LOGDBG1("unhandled type %d\n", type());
        return nullptr;
    }
    return new MetaValue(values);
}

}
}
