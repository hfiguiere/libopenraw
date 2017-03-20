/*
 * libopenraw - mrwcontainer.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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

#include "trace.hpp"
#include "mrwcontainer.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

namespace MRW {

DataBlock::DataBlock(off_t start, MRWContainer *_container)
    : m_start(start), m_container(_container), m_loaded(false)
{
    LOGDBG2("> DataBlock start == %ld\n", start);
    if (m_container->fetchData(m_name, m_start, 4) != 4) {
        // FIXME: Handle error
        LOGWARN("  Error reading block name %ld\n", start);
        return;
    }
    auto result = m_container->readInt32(m_container->file());
    if (result.empty()) {
        // FIXME: Handle error
        LOGWARN("  Error reading block length %ld\n", start);
        return;
    }
    m_length = result.unwrap();
    LOGDBG1("  DataBlock %s, length %d at %ld\n", name().c_str(), m_length, m_start);
    LOGDBG2("< DataBlock\n");
    m_loaded = true;
}

Option<int8_t>
DataBlock::int8_val(off_t off)
{
    MRWContainer *mc = m_container;
    mc->file()->seek(m_start + DataBlockHeaderLength + off, SEEK_SET);
    return mc->readInt8(mc->file());
}

Option<uint8_t>
DataBlock::uint8_val(off_t off)
{
    MRWContainer *mc = m_container;
    mc->file()->seek(m_start + DataBlockHeaderLength + off, SEEK_SET);
    return  mc->readUInt8(mc->file());
}

Option<uint16_t>
DataBlock::uint16_val(off_t off)
{
    MRWContainer *mc = m_container;
    mc->file()->seek(m_start + DataBlockHeaderLength + off, SEEK_SET);
    return mc->readUInt16(mc->file());
}

Option<std::string>
DataBlock::string_val(off_t off)
{
    char buf[9];
    size_t s;
    MRWContainer *mc = m_container;
    s = mc->fetchData(buf, m_start + DataBlockHeaderLength + off, 8);
    if (s != 8) {
        return Option<std::string>();
    }
    buf[8] = 0;
    return Option<std::string>(buf);
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

        LOGDBG1("Identified MRW file\n");

        return ENDIAN_BIG;
    }

    LOGDBG1("Unidentified MRW file\n");

    return ENDIAN_NULL;
}

bool MRWContainer::locateDirsPreHook()
{
    char version[9];
    off_t position;

    LOGDBG1("> MRWContainer::locateDirsPreHook()\n");
    m_endian = ENDIAN_BIG;

    /* MRW file always starts with an MRM datablock. */
    mrm = std::make_shared<MRW::DataBlock>(m_offset, this);
    if (mrm->name() != "MRM") {
        LOGWARN("MRW file begins not with MRM block, "
                "but with unrecognized DataBlock :: name == %s\n",
                mrm->name().c_str());
        return false;
    }

    /* Subblocks are contained within the MRM block. Scan them and create
     * appropriate block descriptors.
     */
    position = mrm->offset() + MRW::DataBlockHeaderLength;
    while (position < pixelDataOffset()) {
        auto ref = std::make_shared<MRW::DataBlock>(position, this);
        LOGDBG1("Loaded DataBlock :: name == %s\n", ref->name().c_str());
        if (!ref || !ref->loaded()) {
            break;
        }
        if (ref->name() == "PRD") {
            if (prd) {
                LOGWARN("File contains duplicate DataBlock :: name == %s\n",
                        ref->name().c_str());
            }
            prd = ref;
        } else if (ref->name() == "TTW") {
            if (ttw) {
                LOGWARN("File contains duplicate DataBlock :: name == %s\n",
                        ref->name().c_str());
            }
            ttw = ref;
        } else if (ref->name() == "WBG") {
            if (wbg) {
                LOGWARN("File contains duplicate DataBlock :: name == %s\n",
                        ref->name().c_str());
            }
            wbg = ref;
        } else if (ref->name() == "RIF") {
            if (rif) {
                LOGWARN("File contains duplicate DataBlock :: name == %s\n",
                        ref->name().c_str());
            }
            rif = ref;
        } else if (ref->name() != "PAD") {
            LOGWARN("File contains unrecognized DataBlock :: name == %s\n",
                    ref->name().c_str());
        }
        position = ref->offset() + MRW::DataBlockHeaderLength + ref->length();
    }

    /* Check that we found all the expected data blocks. */
    if (!prd) {
        LOGWARN("File does NOT contain expected DataBlock :: name == PRD\n");
        return false;
    }
    if (!ttw) {
        LOGWARN("File does NOT contain expected DataBlock :: name == TTW\n");
        return false;
    }
    if (!wbg) {
        LOGWARN("File does NOT contain expected DataBlock :: name == WBG\n");
        return false;
    }
    if (!rif) {
        LOGWARN("File does NOT contain expected DataBlock :: name == RIF\n");
        return false;
    }

    /* Extract the file version string. */
    if (fetchData(version,
                  prd->offset() + MRW::DataBlockHeaderLength + MRW::PRD_VERSION,
                  8) != 8) {
        // FIXME: Handle error
        LOGDBG1("  Error reading version string\n");
    }
    version[8] = '\0';
    m_version = std::string(version);
    LOGDBG1("  MRW file version == %s\n", m_version.c_str());

    /* For the benefit of our parent class, set the container offset to the
     * beginning of
     * the TIFF data (the contents of the TTW data block), and seek there.
     */
    m_offset = ttw->offset() + MRW::DataBlockHeaderLength;

    // TODO: Not sure exactly here the origin of this.
    // But it doesn't work.
    //  if((version[2] != '7') || (version[3] != '3')) {
    setExifOffsetCorrection(m_offset);
    LOGDBG1("setting correction to %ld\n", m_offset);
    //  }

    m_file->seek(m_offset, SEEK_SET);
    LOGDBG1("< MRWContainer\n");

    return true;
}

}
}
