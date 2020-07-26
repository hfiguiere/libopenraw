/* -*- Mode: C++ -*- */
/*
 * libopenraw - ifddir.hpp
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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <map>
#include <memory>
#include <vector>

#include <libopenraw/debug.h>

#include "ifdentry.hpp"
#include "trace.hpp"
#include "option.hpp"
#include "exif/exif_tags.hpp"

namespace OpenRaw {
namespace Internals {

typedef or_ifd_dir_type IfdDirType;

class IfdFileContainer;

class IfdDir {
public:
    typedef std::weak_ptr<IfdDir> WeakRef;
    typedef std::shared_ptr<IfdDir> Ref;
    typedef std::vector<Ref> RefVec;
    typedef std::map<uint16_t, IfdEntry::Ref> Entries;

    IfdDir(off_t _offset, const IfdFileContainer& _container, IfdDirType _type /*= OR_IFDDIR_OTHER */, const TagTable& tag_table = exif_tag_names);
    virtual ~IfdDir();

    IfdDirType type() const
        { return m_type; }
    void setType(IfdDirType type_)
        { m_type = type_; }

    bool isPrimary() const;
    bool isThumbnail() const;

    /** return the offset */
    off_t offset() const { return m_offset; }
    const IfdFileContainer &container() const { return m_container; }

    /** load the directory to memory */
    bool load();
    /** return the number of entries*/
    int numTags() { return m_entries.size(); }
    IfdEntry::Ref getEntry(uint16_t id) const;
    /** Direct access to the entries */
    const Entries& entries() const
        {
            return m_entries;
        }

    /** Get a T value from an entry
     * @param id the IFD field id
     * @return an Option<T> containing the value or none.
     */
    template <typename T>
    Option<T> getValue(uint16_t id) const
    {
        IfdEntry::Ref e = getEntry(id);
        if (e != NULL) {
            try {
                return Option<T>(IfdTypeTrait<T>::get(*e));
            }
            catch (const std::exception &ex) {
                LOGERR("Exception raised %s fetch value for %u\n", ex.what(), id);
            }
        }
        return Option<T>();
    }

    /** Get an loosely typed integer value from an entry.
     * This method is  preferred over getLongValue()
     * or getShortValue() unless you really want the strong
     * typing that IFD structure provide
     * @param id the IFD field id
     * @return an Option<uint32_t> containing the value or none.
     */
    Option<uint32_t> getIntegerValue(uint16_t id);

    /** get the offset of the next IFD
     * in absolute
     */
    off_t nextIFD();

    /** get the SubIFD at index idx.
     * @return Ref to the new IfdDir if found
     */
    Ref getSubIFD(uint32_t idx = 0) const;

    /** get all SubIFDs
     * @return an option of ifds the list of IFDs Ref
     */
    Option<std::vector<IfdDir::Ref>> getSubIFDs();

    /** get the Exif IFD.
     * @return Ref to the new IfdDir if found
     */
    Ref getExifIFD();

    /** get the MakerNote IFD.
     * @param file_type the file type as a hint
     * @return Ref to the new MakerNoteDir if found
     */
    Ref getMakerNoteIfd(or_rawfile_type file_type);

    void setTagTable(const TagTable& tag_table)
        {
            m_tag_table = &tag_table;
        }
    /** Return the tag name for tag
     * @return a static string or nullptr if not found.
     */
    const char* getTagName(uint32_t tag) const;
private:
    IfdDirType m_type;
    off_t m_offset;
    const IfdFileContainer& m_container;
    Entries m_entries;
    const TagTable* m_tag_table;
};
}
}
