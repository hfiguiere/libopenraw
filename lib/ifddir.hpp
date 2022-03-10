/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ifddir.hpp
 *
 * Copyright (C) 2006-2022 Hubert Figuière
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

/** @addtogroup ifd_parsing
 * @{
 */

typedef or_ifd_dir_type IfdDirType;

class IfdFileContainer;

/** @brief An IFD directory */
class IfdDir {
public:
    /** @brief Weak ptr of an IfdDir */
    typedef std::weak_ptr<IfdDir> WeakRef;
    /** @brief Shared ptr of an IfdDir */
    typedef std::shared_ptr<IfdDir> Ref;
    /** @brief Vector of shared ptr of IfdDir
     *
     * Used for situations like with IfdFileContainer.
     */
    typedef std::vector<Ref> RefVec;
    /** @brief Ifd Entries map type.
     *
     * The key is the tag id of the entry.
     */
    typedef std::map<uint16_t, IfdEntry::Ref> Entries;

    /** @brief Construct an IfdDir
     * @param _offset The offset from the begining of the container (where it starts)
     * @param _container The container
     * @param _type The IFD type.
     * @param tag_table The tag to name table. By default it is the Exif tags.
     */
    IfdDir(off_t _offset, RawContainer& _container, IfdDirType _type /*= OR_IFDDIR_OTHER */, const TagTable& tag_table = exif_tag_names);
    virtual ~IfdDir();

    /** @brief Get the type of the IfdDir */
    IfdDirType type() const
        { return m_type; }
    /** @brief Set the type of the IfdDir */
    void setType(IfdDirType type_)
        { m_type = type_; }

    /** @brief Get the base offset for the data
     *
     * Usually it is 0.
     */
    off_t baseOffset() const
        { return m_base_offset; }
    /** @brief Set the base offset */
    void setBaseOffset(off_t base)
        { m_base_offset = base; }

    /** @brief The IFD is a primary in the TIFF/EP sense
     *
     * @return true if EXIF_TAG_NEW_SUBFILE_TYPE is 0.
     */
    bool isPrimary() const;
    /** @brief The IFD is for a thumnail
     *
     * @return true if EXIF_TAG_NEW_SUBFILE_TYPE is 1.
     */
    bool isThumbnail() const;

    /** @brief Return the offset */
    off_t offset() const { return m_offset; }
    /** @brief The container for the IfdDir, const. */
    const RawContainer& container() const { return m_container; }
    /** @brief The container for the IfdDir */
    RawContainer& container() { return m_container; }

    /** @brief Load the directory to memory
     *
     * The only reason you'd want to override is to synthesize an IFD from
     * non-IFD.
     * @return true on success.
     */
    virtual bool load();
    /** @brief Return the number of entries*/
    int numTags() { return m_entries.size(); }
    /** @brief Get the IfdEntry for the tag id
     *
     * Requires load() to have been called once.
     * @return an IfdEntry::Ref. NULL if not found.
     */
    IfdEntry::Ref getEntry(uint16_t id) const;
    /** @brief Direct access to the entries */
    const Entries& entries() const
        {
            return m_entries;
        }
    /** @brief the Container endian
     *
     * Usually it is the same as the file, but MakerNote are weird
     * and might have a different idea.
     */
    RawContainer::EndianType endian() const
        {
            return m_endian;
        }
    /** @brief Set the endian for the IFD.
     *
     * By default it's the same as the container but you might want to set it if,
     * for example, parsing a MakerNote.
     */
    void setEndian(RawContainer::EndianType _endian)
        {
            m_endian = _endian;
        }

    /** @brief Get a T value from an entry
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

    /** @brief Get an loosely typed integer value from an entry.
     *
     * This method is  preferred over getLongValue()
     * or getShortValue() unless you really want the strong
     * typing that IFD structure provide.
     * @param id the IFD field id
     * @return an Option<uint32_t> containing the value or none.
     */
    Option<uint32_t> getIntegerValue(uint16_t id);

    /** @brief Get the offset of the next IFD
     * in absolute
     */
    off_t nextIFD();

    /** @brief Get the SubIFD at index idx.
     * @return Ref to the new IfdDir if found
     */
    Ref getSubIFD(uint32_t idx = 0) const;

    /** @brief Get all SubIFDs
     * @return an option of ifds the list of IFDs Ref
     */
    Option<std::vector<IfdDir::Ref>> getSubIFDs();

    /** @brief Get the Exif IFD.
     * @return Ref to the new IfdDir if found
     */
    Ref getExifIFD();

    /** @brief Get the MakerNote IFD.
     * @param file_type the file type as a hint
     * @return Ref to the new MakerNoteDir if found
     */
    Ref getMakerNoteIfd(or_rawfile_type file_type);

    /** @brief Set the tag table for tag to name correspondance
     *
     * This is used to override the tag name in IFD that use non standard
     * tags, like MakerNote or Panasonic RW2.
     */
    void setTagTable(const TagTable& tag_table)
        {
            m_tag_table = &tag_table;
        }
    /** @brief Return the tag name for tag
     * @return a static string or nullptr if not found.
     */
    const char* getTagName(uint32_t tag) const;

    /** @brief Get the entry value as an array */
    template <typename T>
    Option<std::vector<T>> getEntryArrayValue(IfdEntry& e) const;
    /** @brief Get the typed entry value */
    template<typename T>
    T getEntryValue(IfdEntry& e, uint32_t idx = 0, bool ignore_type = false) const;

    /** @brief Return the integer value at index. It will coerce the type.
     * @param e the IFD entry
     * @param idx the index
     * @return the integer value or 0.
     */
    uint32_t getEntryIntegerArrayItemValue(IfdEntry& e, int idx) const;

    /** @brief Make a meta value out of the IFD entry.
     * @return a %MetaValue or nullptr. Must be freed.
     */
    MetaValue* makeMetaValue(IfdEntry& e) const;
protected:
    /** The IFD entries */
    Entries m_entries;
private:
    IfdDirType m_type;
    off_t m_offset;
    RawContainer& m_container;
    const TagTable* m_tag_table;
    off_t m_base_offset;
    RawContainer::EndianType m_endian;
};


/** @brief Get the array values of type T
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

/** @brief Get the value of type T
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

/** @} */
}
}
