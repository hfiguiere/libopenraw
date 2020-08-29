/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ciffifd.hpp
 *
 * Copyright (C) 2020 Hubert Figuière
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

#include "ifddir.hpp"

namespace OpenRaw {
namespace Internals {

class CRWFile;
class RawContainer;

namespace CIFF {

class CiffIfd
    : public IfdDir
{
public:
    CiffIfd(CRWFile& ciff, RawContainer& container, IfdDirType _type);
    IfdEntry::Ref entryForString(uint16_t id, const std::string& str) const;
protected:
    CRWFile& m_file;
};

class CiffMainIfd
    : public CiffIfd
{
public:
    CiffMainIfd(CRWFile& ciff, RawContainer& container);
    virtual bool load() override;
};

class CiffExifIfd
    : public CiffIfd
{
public:
    CiffExifIfd(CRWFile& ciff, RawContainer& container);
    virtual bool load() override;
};

}
}
}
