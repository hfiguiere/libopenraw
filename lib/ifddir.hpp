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

    IfdDir(off_t _offset, const RawContainer& _container, IfdDirType _type /*= OR_IFDDIR_OTHER */, const TagTable& tag_table = exif_tag_names);
    virtual ~IfdDir();

    IfdDirType type() const
        { return m_type; }
    void setType(IfdDirType type_)
        { m_type = type_; }

    off_t baseOffset() const
        { return m_base_offset; }
    void setBaseOffset(off_t base)
        { m_base_offset = base; }

    bool isPrimary() const;
    bool isThumbnail() const;

    /** return the offset */
    off_t offset() const { return m_offset; }
    const RawContainer& container() const { return m_container; }

    /** load the directory to memory
     * The only reason you'd want to override is to synthesize an IFD from
     * non-IFD.
     * @return true on success.
     */
    virtual bool load();
    /** return the number of entries*/
    int numTags() { return m_entries.size(); }
    IfdEntry::Ref getEntry(uint16_t id) const;
    /** Direct access to the entries */
    const Entries& entries() const
        {
            return m_entries;
        }
    RawContainer::EndianType endian() const
        {
            return m_endian;
        }
    /** Set the endian for the IFD. By default it's the same as the container */
    void setEndian(RawContainer::EndianType _endian)
        {
            m_endian = _endian;
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
                return Option<T>(getEntryValue<T>(*e));
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

    /** Get the entry value as an array */
    template <typename T>
    Option<std::vector<T>> getEntryArrayValue(IfdEntry& e) const;
    /** Get the typed entry value */
    template<typename T>
    T getEntryValue(IfdEntry& e, uint32_t idx = 0, bool ignore_type = false) const;

    /** copy the enty data. Endian is ignored. Suite for bytes, undefined, etc
     * @param e the entry
     * @param buffer the buffer
     * @param buffersize the size of the buffer in bytes. Will copy at most this.
     * @return the number of bytes copied
     */
    size_t getEntryData(IfdEntry& e, uint8_t* buffer, size_t buffersize) const;

    uint32_t getEntryIntegerArrayItemValue(IfdEntry& e, int idx) const;

    MetaValue* makeMetaValue(IfdEntry& e) const;
protected:
    Entries m_entries;
private:
    IfdDirType m_type;
    off_t m_offset;
    const RawContainer& m_container;
    const TagTable* m_tag_table;
    off_t m_base_offset;
    RawContainer::EndianType m_endian;
};


/** get the array values of type T
 * @param T the type of the value needed
 * @param array the storage
 * @throw whatever is thrown
 */
template <typename T>
Option<std::vector<T>> IfdDir::getEntryArrayValue(IfdEntry& entry) const
{
    try {
        std::vector<T> array;
        array.reserve(entry.count());
        for (uint32_t i = 0; i < entry.count(); i++) {
            array.push_back(getEntryValue<T>(entry, i));
        }
        return Option<decltype(array)>(array);
    }
    catch(const std::exception& e)
    {
        LOGERR("Exception: %s\n", e.what());
    }
    return OptionNone();
}

/** get the value of type T
 * @param T the type of the value needed
 * @param idx the index, by default 0
 * @param ignore_type if true, don't check type. *DANGEROUS* Default is false.
 * @return the value
 * @throw BadTypeException in case of wrong typing.
 * @throw OutOfRangeException in case of subscript out of range
 */
template <typename T>
T IfdDir::getEntryValue(IfdEntry& e, uint32_t idx, bool ignore_type) const
	noexcept(false)
{
    /* format undefined means that we don't check the type */
    if(!ignore_type && (e.type() != IFD::EXIF_FORMAT_UNDEFINED)) {
        if (e.type() != IfdTypeTrait<T>::type) {
            throw BadTypeException();
        }
    }
    if (idx + 1 > e.count()) {
        throw OutOfRangeException();
    }

    if (!e.loadData(IfdTypeTrait<T>::size, m_base_offset)) {
        throw TooBigException();
    }

    const uint8_t *data = e.dataptr();
    data += (IfdTypeTrait<T>::size * idx);
    T val;
    if (e.endian() == RawContainer::ENDIAN_LITTLE) {
        val = IfdTypeTrait<T>::EL(data, e.count() - idx);
    } else {
        val = IfdTypeTrait<T>::BE(data, e.count() - idx);
    }
    return val;
}


}
}
