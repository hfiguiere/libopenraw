/*
 * libopenraw - metadata.cpp
 *
 * Copyright (C) 2020 Hubert Figuière
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

#include "trace.hpp"
#include "rawfile.hpp"
#include "metadata.hpp"

namespace OpenRaw {

MetadataIterator::MetadataIterator(RawFile& rf)
    : m_is_initialized(false)
    , m_is_valid(true)
    , m_next_ifd(0)
{
    auto ifd = rf.exifIfd();
    if (ifd) {
        m_ifds.push_back(ifd);
    }
    ifd = rf.makerNoteIfd();
    if (ifd) {
        m_ifds.push_back(ifd);
    }
}

Internals::IfdDir::Ref MetadataIterator::nextIfd()
{
    m_is_initialized = true;
    if (m_ifds.size() <= m_next_ifd) {
        return Internals::IfdDir::Ref();
    }
    auto ifd = m_ifds[m_next_ifd];
    m_next_ifd++;
    m_current_entry = ifd->entries().begin();
    return ifd;
}

bool MetadataIterator::next()
{
    if (!m_is_valid) {
        LOGDBG1("Invalid iterator\n");
        return false;
    }
    LOGDBG1("next\n");
    if (m_current_ifd) {
        m_current_entry++;
        if (m_current_entry == m_current_ifd->entries().end()) {
            LOGDBG1("end of IFD, moving on\n");
            m_current_ifd = nextIfd();
        }
    } else {
        m_current_ifd = nextIfd();
    }
    if (!m_current_ifd) {
        m_is_valid = false;
        LOGDBG1("no more current ifd\n");
        return false;
    }

    return true;
}

/// Get the type of the current entry
Option<ExifTagType> MetadataIterator::getEntryType() const
{
    if (!(isInitialized() && isValid())) {
        return OptionNone();
    }
    return static_cast<ExifTagType>(m_current_entry->second->type());
}

/// Get the ID of the current entry
Option<uint16_t> MetadataIterator::getEntryId() const
{
    if (!(isInitialized() && isValid())) {
        return OptionNone();
    }
    return m_current_entry->first;
}

}
