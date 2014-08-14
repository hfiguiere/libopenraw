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

#include <string.h>

#include <libopenraw/rawfile.h>

#include "makernotedir.h"
#include "io/stream.h"
#include "ifdfilecontainer.h"

namespace OpenRaw {
namespace Internals {

MakerNoteDir::Ref
MakerNoteDir::createMakerNote(off_t offset,
                              IfdFileContainer & container)
{
    char data[18];
    auto file = container.file();
    file->seek(offset, SEEK_SET);
    file->read(&data, 18);

    if (memcmp("Nikon\0", data, 6) == 0) {
        if (data[6] == 1) {
            return std::make_shared<MakerNoteDir>(
                offset + 8, container, offset + 8);
        }
        else if (data[6] == 2) {
            // this one has an endian / TIFF header after the magic
            return std::make_shared<MakerNoteDir>(
                offset + 18, container, offset + 10);
        }
        else {
            return std::make_shared<MakerNoteDir>(
                offset, container, offset);
        }
    }

    if (memcmp("OLYMPUS\0", data, 8) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 12, container, offset);
    }

    if (memcmp("OLYMP\0", data, 6) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, offset + 8);
    }

#if 0 // Minolta stub. Untested.
    if (memcmp("MLT0\0", data + 10, 5) == 0) {
        return std::make_shared<MakerNoteDir>(
            offset + 8, container, offset + 8);
    }
#endif


    return std::make_shared<MakerNoteDir>(offset, container, offset);
}

MakerNoteDir::MakerNoteDir(off_t _offset,
                           IfdFileContainer & _container,
                           off_t mnote_offset)
    : MakerNoteDir("", 0, _offset, _container, mnote_offset)
{
}

MakerNoteDir::MakerNoteDir(const char* magic, size_t hlen,
                           off_t _offset,
                           IfdFileContainer & _container,
                           off_t mnote_offset)
    : IfdDir(_offset, _container)
    , m_magic(magic ? magic : "")
    , m_hlen(hlen)
    , m_mnote_offset(mnote_offset)
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
