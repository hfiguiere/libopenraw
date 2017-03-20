/*
 * libopenraw - ifdfilecontainer.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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
#include <sys/types.h>
#include <memory>

#include <vector>

#include <libopenraw/debug.h>

#include "trace.hpp"
#include "ifdfilecontainer.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

IfdFileContainer::IfdFileContainer(const IO::Stream::Ptr &_file, off_t _offset)
    : RawContainer(_file, _offset)
    , m_error(0)
    , m_exif_offset_correction(0)
    , m_current_dir()
    , m_dirs()
{
}

IfdFileContainer::~IfdFileContainer()
{
    m_dirs.clear();
}

IfdFileContainer::EndianType IfdFileContainer::isMagicHeader(const char *p,
                                                             int len)
{
    if (len < 4) {
        // we need at least 4 bytes to check
        return ENDIAN_NULL;
    }
    if ((p[0] == 0x49) && (p[1] == 0x49) && (p[2] == 0x2a) && (p[3] == 0x00)) {
        return ENDIAN_LITTLE;
    } else if ((p[0] == 0x4d) && (p[1] == 0x4d) && (p[2] == 0x00) &&
               (p[3] == 0x2a)) {
        return ENDIAN_BIG;
    }
    return ENDIAN_NULL;
}

int IfdFileContainer::countDirectories(void)
{
    if (m_dirs.size() == 0) {
        // FIXME check result
        bool ret = _locateDirs();
        if (!ret) {
            return -1;
        }
    }
    return m_dirs.size();
}

std::vector<IfdDir::Ref> &IfdFileContainer::directories()
{
    if (m_dirs.size() == 0) {
        countDirectories();
    }
    return m_dirs;
}

IfdDir::Ref IfdFileContainer::setDirectory(int dir)
{
    if (dir < 0) {
        // FIXME set error
        return IfdDir::Ref();
    }
    // FIXME handle negative values
    int n = countDirectories();
    if (n <= 0) {
        // FIXME set error
        return IfdDir::Ref();
    }
    // dir is signed here because we can pass negative
    // value for specific Exif IFDs.
    if (dir > (int)m_dirs.size()) {
        // FIXME set error
        return IfdDir::Ref();
    }
    m_current_dir = m_dirs[dir];
    m_current_dir->load();
    return m_current_dir;
}

size_t IfdFileContainer::getDirectoryDataSize()
{
    // TODO move to IFDirectory
    LOGDBG1("getDirectoryDataSize()\n");
    off_t dir_offset = m_current_dir->offset();
    // FIXME check error
    LOGDBG1("offset = %ld m_numTags = %d\n", dir_offset, m_current_dir->numTags());
    off_t begin = dir_offset + 2 + (m_current_dir->numTags() * 12);

    LOGDBG1("begin = %ld\n", begin);

    m_file->seek(begin, SEEK_SET);
    begin += 2;

    int32_t nextIFD = readInt32(m_file).unwrap_or(0);
    LOGDBG1("nextIFD = %d\n", nextIFD);
    if (nextIFD == 0) {
        // FIXME not good
        // XXX we should check the Option<> from readInt32().
    }
    return nextIFD - begin;
}

bool IfdFileContainer::locateDirsPreHook()
{
    return true;
}

bool IfdFileContainer::_locateDirs(void)
{
    if (!locateDirsPreHook()) {
        return false;
    }
    LOGDBG1("_locateDirs()\n");
    if (m_endian == ENDIAN_NULL) {
        char buf[4];
        m_file->seek(m_offset, SEEK_SET);
        m_file->read(buf, 4);
        m_endian = isMagicHeader(buf, 4);
        if (m_endian == ENDIAN_NULL) {
            // FIXME set error code
            return false;
        }
    }
    m_file->seek(m_offset + 4, SEEK_SET);
    int32_t dir_offset = readInt32(m_file).unwrap_or(0);
    m_dirs.clear();
    do {
        if (dir_offset != 0) {
            LOGDBG1("push offset =0x%x\n", dir_offset);

            // we assume the offset is relative to the begining of
            // the IFD.
            IfdDir::Ref dir(
                std::make_shared<IfdDir>(m_offset + dir_offset, *this));
            m_dirs.push_back(dir);

            dir_offset = dir->nextIFD();
        }
    } while (dir_offset != 0);

    LOGDBG1("# dir found = %ld\n", m_dirs.size());
    return (m_dirs.size() != 0);
}
}
}
