/*
 * libopenraw - makernotedir.cpp
 *
 * Copyright (C) Hubert Figuiere
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

/*
 * For Makernote detection, see:
 *   http://owl.phy.queensu.ca/~phil/exiftool/makernote_types.html
 *   http://www.exiv2.org/makernote.html
 */
MakerNoteDir::Ref
MakerNoteDir::createMakerNote(off_t offset,
                              IfdFileContainer & container)
{
    LOGDBG1("createMakerNote()\n");
    char data[18];
    auto file = container.file();
    file->seek(offset, SEEK_SET);
    file->read(&data, 18);

    if (memcmp("Nikon\0", data, 6) == 0) {
        if (data[6] == 1) {
            return std::make_shared<MakerNoteDir>(
                offset + 8, container, offset + 8, "Nikon2");
        }
        else if (data[6] == 2) {
            // this one has an endian / TIFF header after the magic
            return std::make_shared<MakerNoteDir>(
                offset + 18, container, offset + 10, "Nikon");
        }
        else {
            return std::make_shared<MakerNoteDir>(
                offset, container, offset, "");
        }
    }

    if (memcmp("OLYMPUS\0", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset, "Olympus2");
    }

    if (memcmp("OLYMP\0", data, 6) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, offset + 8, "Olympus");
    }

    if (memcmp("MLT0", data + 10, 4) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset, container, offset, "Minolta");
    }

    return std::make_shared<MakerNoteDir>(offset, container, offset, "");
}

MakerNoteDir::MakerNoteDir(off_t _offset,
                           IfdFileContainer & _container,
                           off_t mnote_offset,
                           const std::string & id)
    : MakerNoteDir("", 0, _offset, _container, mnote_offset, id)
{
}

MakerNoteDir::MakerNoteDir(const char* magic, size_t hlen,
                           off_t _offset,
                           IfdFileContainer & _container,
                           off_t mnote_offset,
                           const std::string & id)
    : IfdDir(_offset, _container)
    , m_magic(magic ? magic : "")
    , m_hlen(hlen)
    , m_mnote_offset(mnote_offset)
    , m_id(id)
{
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
