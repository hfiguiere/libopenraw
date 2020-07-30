/*
 * libopenraw - makernotedir.cpp
 *
 * Copyright (C) 2010-2020 Hubert Figuière
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

#include <string.h>

#include "makernotedir.hpp"
#include "io/stream.hpp"
#include "ifdfilecontainer.hpp"
#include "trace.hpp"

namespace OpenRaw {
namespace Internals {

namespace {
const TagTable empty_tag_names = {};
}

/*
 * For Makernote detection, see:
 *   http://owl.phy.queensu.ca/~phil/exiftool/makernote_types.html
 *   https://exiftool.org/makernote_types.html
 *   http://www.exiv2.org/makernote.html
 */
MakerNoteDir::Ref
MakerNoteDir::createMakerNote(off_t offset,
                              const IfdFileContainer& container, or_rawfile_type file_type)
{
    LOGDBG1("createMakerNote()\n");

    if (file_type == OR_RAWFILE_TYPE_CR2
        || file_type == OR_RAWFILE_TYPE_CR3
        || file_type == OR_RAWFILE_TYPE_CRW) {

        return std::make_shared<MakerNoteDir>(
            offset, container, 0, "Canon", mnote_canon_tag_names);
    }

    char data[18];
    auto file = container.file();
    file->seek(offset, SEEK_SET);
    file->read(&data, 18);
    // LOGDBG1("data %s\n", Debug::bytes_to_string((uint8_t*)&data, 18).c_str());

    if (memcmp("Nikon\0", data, 6) == 0) {
        if (data[6] == 1) {
            return std::make_shared<MakerNoteDir>(
                offset + 8, container, offset + 8, "Nikon2", mnote_nikon2_tag_names);
        }
        else if (data[6] == 2) {
            // this one has an endian / TIFF header after the magic
            return std::make_shared<MakerNoteDir>(
                offset + 18, container, offset + 10, "Nikon", mnote_nikon_tag_names);
        }
        else {
            return std::make_shared<MakerNoteDir>(
                offset, container, offset, "", empty_tag_names);
        }
    }

    if (memcmp("OLYMPUS\0", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset, "Olympus2", mnote_olympus_tag_names);
    }

    if (memcmp("OLYMP\0", data, 6) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, offset + 8, "Olympus", mnote_olympus_tag_names);
    }
    // EPSON R-D1, use Olympus
    // XXX deal with endian.
    if (memcmp("EPSON\0", data, 6) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, 0, "Olympus", mnote_olympus_tag_names);
    }

    // Pentax Asahi Optical Corporation (pre Ricoh merger)
    if (memcmp("AOC\0", data, 4) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 6, container, 0, "Pentax", mnote_pentax_tag_names);
    }
    // Pentax post Ricoh merger
    if (memcmp("PENTAX \0", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 10, container, offset, "Pentax", mnote_pentax_tag_names);
    }

    if (memcmp("SONY DSC \0", data, 10) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset + 12, "Panasonic", mnote_panasonic_tag_names);
    }

    if (memcmp("Panasonic\0", data, 10) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, 0, "Panasonic", mnote_panasonic_tag_names);
    }

    if (memcmp("FUJIFILM", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset, "Fujifilm", mnote_fujifilm_tag_names);
    }

    if (memcmp("MLT0", data + 10, 4) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset, container, offset, "Minolta", mnote_minolta_tag_names);
    }

    return std::make_shared<MakerNoteDir>(offset, container, offset, "", empty_tag_names);
}

MakerNoteDir::MakerNoteDir(off_t _offset,
                           const IfdFileContainer& _container,
                           off_t mnote_offset,
                           const std::string& id,
                           const TagTable& tag_table)
    : MakerNoteDir("", 0, _offset, _container, mnote_offset, id, tag_table)
{
}

MakerNoteDir::MakerNoteDir(const char* magic, size_t hlen,
                           off_t _offset,
                           const IfdFileContainer& _container,
                           off_t mnote_offset,
                           const std::string& id,
                           const TagTable& tag_table)
    : IfdDir(_offset, _container, OR_IFD_MNOTE, tag_table)
    , m_magic(magic ? magic : "")
    , m_hlen(hlen)
    , m_mnote_offset(mnote_offset)
    , m_id(id)
{
    setBaseOffset(mnote_offset);
}

MakerNoteDir::~MakerNoteDir()
{
}

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
