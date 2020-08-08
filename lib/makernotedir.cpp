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

    // Canon MakerNote don't have an ID
    if (file_type == OR_RAWFILE_TYPE_CR2
        || file_type == OR_RAWFILE_TYPE_CR3
        || file_type == OR_RAWFILE_TYPE_CRW) {

        return std::make_shared<MakerNoteDir>(
            offset, container, 0, "Canon", mnote_canon_tag_names);
    }

    // Sony RAW MakerNote don't have an ID
    if (file_type == OR_RAWFILE_TYPE_ARW) {
        return std::make_shared<MakerNoteDir>(
            offset, container, 0, "Sony5", mnote_sony_tag_names);
    }

    uint8_t data[18];
    auto file = container.file();
    file->seek(offset, SEEK_SET);
    file->read(&data, 18);
    // LOGDBG1("data %s\n", Debug::ascii_to_string(data, 18).c_str());
    // LOGDBG1("data %s\n", Debug::bytes_to_string(data, 18).c_str());

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
    // Headerless Nikon.
    if (file_type == OR_RAWFILE_TYPE_NEF) {
        return std::make_shared<MakerNoteDir>(
            offset, container, offset, "Nikon", mnote_nikon_tag_names);
    }

    if (memcmp("OLYMPUS\0", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset, "Olympus2", mnote_olympus_tag_names);
    }

    if (memcmp("OLYMP\0", data, 6) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, 0, "Olympus", mnote_olympus_tag_names);
    }
    // EPSON R-D1, use Olympus
    // XXX deal with endian.
    if (memcmp("EPSON\0", data, 6) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, 0, "Epson", mnote_olympus_tag_names);
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

    if (memcmp("Panasonic\0", data, 10) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, 0, "Panasonic", mnote_panasonic_tag_names);
    }

    if (memcmp("Ricoh\0", data, 5) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, 0, "Ricoh", mnote_ricoh_tag_names);
    }

    if (memcmp("LEICA", data, 5) == 0) {
        if (data[5] == 0 && data[6] == 0 && data[7] == 0) {
            if (file_type == OR_RAWFILE_TYPE_RW2) {
                // Panasonic
                return std::make_shared<MakerNoteDir>(
                    offset + 8, container, 0, "Panasonic", mnote_panasonic_tag_names);
            } else {
                // Leica M8
                return std::make_shared<MakerNoteDir>(
                    offset + 8, container, offset, "Leica2", mnote_leica2_tag_names);
            }
        }

        if (data[5] == 0 && data[7] == 0) {
            switch (data[6]) {
            case 0x08:
            case 0x09:
                // Leica Q Typ 116 and SL (Type 601)
                return std::make_shared<MakerNoteDir>(
                    offset + 8, container, 0, "Leica5", mnote_leica5_tag_names);
            case 0x01: // Leica X1
            case 0x04: // Leica X VARIO
            case 0x05: // Leica X2
            case 0x06: // Leica T (Typ 701)
            case 0x07: // Leica X (Typ 113)
            case 0x10: // Leica X-U (Typ 113)
            case 0x1a:
                return std::make_shared<MakerNoteDir>(
                    offset + 8, container, offset, "Leica5", mnote_leica5_tag_names);
            }
        }

        // Leica M (Typ 240)
        if (data[5] == 0x0 && data[6] == 0x02 && data[7] == 0xff) {
            return std::make_shared<MakerNoteDir>(
                offset + 8, container, 0, "Leica6", mnote_leica6_tag_names);
        }

        // Leica M9/Monochrom
        if (data[5] == '0' && data[6] == 0x03 && data[7] == 0) {
            return std::make_shared<MakerNoteDir>(
                offset + 8, container, offset, "Leica4", mnote_leica4_tag_names);
        }

        // Leica M10
        if (data[5] == 0 && data[6] == 0x02 && data[7] == 0) {
            return std::make_shared<MakerNoteDir>(
                offset + 8, container, 0, "Leica9", mnote_leica9_tag_names);
        }
    }

    if (memcmp("YI     \0", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset, "Xiaoyi", mnote_xiaoyi_tag_names);
    }

    if (memcmp("Apple iOS\0", data, 10) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 14, container, offset, "Apple", mnote_apple_tag_names);
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

MakerNoteDir::MakerNoteDir(IfdDir& ifd, const std::string& id, const TagTable& tag_table)
    : MakerNoteDir(ifd.offset(), ifd.container(), ifd.offset(), id, tag_table)
{
    setBaseOffset(0);
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

IfdDir::Ref MakerNoteDir::getIfdInEntry(uint16_t id)
{
    auto entry = getEntry(id);
    if (!entry) {
        LOGDBG1("Coudln't get entry %u\n", id);
        return Ref();
    }

    uint32_t val_offset = 0;
    // "INVALID" type entry  (13) for some Olympus MakerNote
    if (entry->type() == 13 || entry->type() == IFD::EXIF_FORMAT_LONG) {
        val_offset = getEntryValue<uint32_t>(*entry, 0, true);
        LOGDBG1("Custom IFD offset (uncorrected) = %u\n", val_offset);
        val_offset += container().exifOffsetCorrection() + getMnoteOffset();
    } else {
        // Type is likely "UNDEFINED"
        val_offset = entry->offset();
    }
    LOGDBG1("Custom IFD offset = %u\n", val_offset);

    auto ref = std::make_shared<IfdDir>(val_offset, container(), OR_IFD_OTHER);
    ref->load();
    return ref;
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
