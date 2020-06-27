/*
 * libopenraw - metadata.hpp
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


#pragma once

#include <vector>

#include "ifddir.hpp"

namespace OpenRaw {

class RawFile;

class MetadataIterator
{
public:
    MetadataIterator(RawFile& rf);
    bool next();
    bool isInitialized() const
        {
            return m_is_initialized;
        }
    bool isValid() const
        {
            return m_is_valid;
        }

    /// Get the type of the current entry
    Option<ExifTagType> getEntryType() const;
    /// Get the ID of the current entry
    Option<uint16_t> getEntryId() const;
    MetaValue* getMetaValue() const;

private:
    Internals::IfdDir::Ref nextIfd();

    /// Safe guard against getting data. False until an IFD is picked.
    bool m_is_initialized;
    /// Safe guard against advancing. True until the end.
    bool m_is_valid;
    /// Index of the next IFD.
    size_t m_next_ifd;
    Internals::IfdDir::Ref m_current_ifd;
    Internals::IfdDir::Entries::const_iterator m_current_entry;
    std::vector<Internals::IfdDir::Ref> m_ifds;
};

}
