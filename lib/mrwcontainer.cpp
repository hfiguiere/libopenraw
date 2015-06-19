/*
 * libopenraw - mrwcontainer.cpp
 *
 * Copyright (C) 2006-2015 Hubert Figuiere
 * Copyright (C) 2008 Bradley Broom
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
#include <stddef.h>

#include <libopenraw/debug.h>

#include "trace.h"
#include "mrwcontainer.h"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

namespace MRW {

DataBlock::DataBlock(off_t start, MRWContainer *_container)
    : m_start(start), m_container(_container), m_loaded(false)
{
    Trace(DEBUG2) << "> DataBlock start == " << start << "\n";
    if (m_container->fetchData(m_name, m_start, 4) != 4) {
        // FIXME: Handle error
        Trace(WARNING) << "  Error reading block name " << start << "\n";
        return;
    }
    if (!m_container->readInt32(m_container->file(), m_length)) {
        // FIXME: Handle error
        Trace(WARNING) << "  Error reading block length " << start << "\n";
        return;
    }
    Trace(DEBUG1) << "  DataBlock " << name() << ", length " << m_length
                  << " at " << m_start << "\n";
    Trace(DEBUG2) << "< DataBlock\n";
    m_loaded = true;
}

int8_t DataBlock::int8_val(off_t off)
{
    int8_t ret;
    MRWContainer *mc = m_container;
    mc->file()->seek(m_start + DataBlockHeaderLength + off, SEEK_SET);
    mc->readInt8(mc->file(), ret);
    return ret;
}

uint8_t DataBlock::uint8_val(off_t off)
{
    uint8_t ret;
    MRWContainer *mc = m_container;
    mc->file()->seek(m_start + DataBlockHeaderLength + off, SEEK_SET);
    mc->readUInt8(mc->file(), ret);
    return ret;
}

uint16_t DataBlock::uint16_val(off_t off)
{
    uint16_t ret;
    MRWContainer *mc = m_container;
    mc->file()->seek(m_start + DataBlockHeaderLength + off, SEEK_SET);
    mc->readUInt16(mc->file(), ret);
    return ret;
}

std::string DataBlock::string_val(off_t off)
{
    char buf[9];
    size_t s;
    MRWContainer *mc = m_container;
    s = mc->fetchData(buf, m_start + DataBlockHeaderLength + off, 8);
    if (s == 8) {
        buf[8] = 0;
    } else {
        *buf = 0;
    }
    return buf;
}
}

MRWContainer::MRWContainer(const IO::Stream::Ptr &_file, off_t _offset)
    : IfdFileContainer(_file, _offset)
{
}

MRWContainer::~MRWContainer()
{
}

IfdFileContainer::EndianType MRWContainer::isMagicHeader(const char *p, int len)
{
    if (len < 4) {
        // we need at least 4 bytes to check
        return ENDIAN_NULL;
    }

    if ((p[0] == 0x00) && (p[1] == 'M') && (p[2] == 'R') && (p[3] == 'M')) {

        Trace(DEBUG1) << "Identified MRW file\n";

        return ENDIAN_BIG;
    }

    Trace(DEBUG1) << "Unidentified MRW file\n";

    return ENDIAN_NULL;
}

bool MRWContainer::locateDirsPreHook()
{
    char version[9];
    off_t position;

    Trace(DEBUG1) << "> MRWContainer::locateDirsPreHook()\n";
    m_endian = ENDIAN_BIG;

    /* MRW file always starts with an MRM datablock. */
    mrm = std::make_shared<MRW::DataBlock>(m_offset, this);
    if (mrm->name() != "MRM") {
        Trace(WARNING) << "MRW file begins not with MRM block, "
                          "but with unrecognized DataBlock :: name == "
                       << mrm->name() << "\n";
        return false;
    }

    /* Subblocks are contained within the MRM block. Scan them and create
     * appropriate block descriptors.
     */
    position = mrm->offset() + MRW::DataBlockHeaderLength;
    while (position < pixelDataOffset()) {
        MRW::DataBlock::Ref ref(
            std::make_shared<MRW::DataBlock>(position, this));
        Trace(DEBUG1) << "Loaded DataBlock :: name == " << ref->name() << "\n";
        if (!ref || !ref->loaded()) {
            break;
        }
        if (ref->name() == "PRD") {
            if (prd) {
                Trace(WARNING)
                    << "File contains duplicate DataBlock :: name == "
                    << ref->name() << "\n";
            }
            prd = ref;
        } else if (ref->name() == "TTW") {
            if (ttw) {
                Trace(WARNING)
                    << "File contains duplicate DataBlock :: name == "
                    << ref->name() << "\n";
            }
            ttw = ref;
        } else if (ref->name() == "WBG") {
            if (wbg) {
                Trace(WARNING)
                    << "File contains duplicate DataBlock :: name == "
                    << ref->name() << "\n";
            }
            wbg = ref;
        } else if (ref->name() == "RIF") {
            if (rif) {
                Trace(WARNING)
                    << "File contains duplicate DataBlock :: name == "
                    << ref->name() << "\n";
            }
            rif = ref;
        } else if (ref->name() != "PAD") {
            Trace(WARNING) << "File contains unrecognized DataBlock :: name == "
                           << ref->name() << "\n";
        }
        position = ref->offset() + MRW::DataBlockHeaderLength + ref->length();
    }

    /* Check that we found all the expected data blocks. */
    if (!prd) {
        Trace(WARNING)
            << "File does NOT contain expected DataBlock :: name == PRD\n";
        return false;
    }
    if (!ttw) {
        Trace(WARNING)
            << "File does NOT contain expected DataBlock :: name == TTW\n";
        return false;
    }
    if (!wbg) {
        Trace(WARNING)
            << "File does NOT contain expected DataBlock :: name == WBG\n";
        return false;
    }
    if (!rif) {
        Trace(WARNING)
            << "File does NOT contain expected DataBlock :: name == RIF\n";
        return false;
    }

    /* Extract the file version string. */
    if (fetchData(version,
                  prd->offset() + MRW::DataBlockHeaderLength + MRW::PRD_VERSION,
                  8) != 8) {
        // FIXME: Handle error
        Debug::Trace(DEBUG1) << "  Error reading version string\n";
    }
    version[8] = '\0';
    m_version = std::string(version);
    Trace(DEBUG1) << "  MRW file version == " << m_version << "\n";

    /* For the benefit of our parent class, set the container offset to the
     * beginning of
     * the TIFF data (the contents of the TTW data block), and seek there.
     */
    m_offset = ttw->offset() + MRW::DataBlockHeaderLength;

    // TODO: Not sure exactly here the origin of this.
    // But it doesn't work.
    //  if((version[2] != '7') || (version[3] != '3')) {
    setExifOffsetCorrection(m_offset);
    Trace(DEBUG1) << "setting correction to " << m_offset << "\n";
    //  }

    m_file->seek(m_offset, SEEK_SET);
    Trace(DEBUG1) << "< MRWContainer\n";

    return true;
}
}
}
