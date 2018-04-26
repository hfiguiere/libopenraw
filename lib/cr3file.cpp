/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - cr3file.cpp
 *
 * Copyright (C) 2018 Hubert Figuière
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

#include <cstdint>
#include <memory>
#include <stddef.h>
#include <vector>

#include <libopenraw/cameraids.h>
#include <libopenraw/consts.h>
#include <libopenraw/debug.h>

#include "cr3file.hpp"
#include "isomediacontainer.hpp"
#include "rawfile_private.hpp"
#include "trace.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_CANON_TYPEID(camid)                                            \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, camid)

/* all relative to the D65 calibration illuminant */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const RawFile::camera_ids_t Cr3File::s_def[] = {
    { "Canon EOS M50", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50) },
    { 0, 0 }
};

RawFile *Cr3File::factory(const IO::Stream::Ptr &s)
{
    return new Cr3File(s);
}

Cr3File::Cr3File(const IO::Stream::Ptr &s)
    : RawFile(OR_RAWFILE_TYPE_CR3)
    , m_io(s)
    , m_container(new IsoMediaContainer(s))
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

Cr3File::~Cr3File() {}

RawContainer *Cr3File::getContainer() const
{
    return m_container;
}

::or_error Cr3File::_getRawData(RawData &data, uint32_t options)
{
    LOGDBG1("Unimplemented\n");
    return OR_ERROR_NOT_FOUND;
}

::or_error Cr3File::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
    auto err = OR_ERROR_NOT_FOUND;

    return err;
}

MetaValue *Cr3File::_getMetaValue(int32_t meta_index)
{
    MetaValue *val = nullptr;
    return val;
}

void Cr3File::_identifyId()
{
    LOGERR("Not implemented\n");
}
}
}
