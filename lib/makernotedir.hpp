/*
 * libopenraw - makernotedir.h
 *
 * Copyright (C) 2010-2020 Hubert Figuiere
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

#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <string>

#include "ifddir.hpp"

namespace OpenRaw {
namespace Internals {

class IfdFileContainer;

class MakerNoteDir
    : public IfdDir
{
public:
    typedef std::shared_ptr<MakerNoteDir> Ref;

    /** Create the appropriate MakerNote at offset */
    static Ref createMakerNote(off_t offset,
                               const IfdFileContainer& container, or_rawfile_type file_type);

    MakerNoteDir(off_t _offset, const IfdFileContainer& _container,
                 off_t mnote_offset, const std::string & id,
                 const TagTable& tag_table);
    virtual ~MakerNoteDir();

    off_t getMnoteOffset() const
        { return m_mnote_offset; }

    const std::string & getId() const
        { return m_id; }
protected:
    MakerNoteDir(const char* magic, size_t hlen,
                 off_t _offset,
                 const IfdFileContainer& _container,
                 off_t mnote_offset,
                 const std::string & id,
                 const TagTable& tag_table);

    std::string m_magic;
    size_t m_hlen;
    off_t m_mnote_offset;
    std::string m_id;
};

}
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
