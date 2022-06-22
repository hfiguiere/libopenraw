/* -*- Mode: C++; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * libopenraw - orffile.hpp
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

#include <stdint.h>
#include <string>
#include <vector>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifd.hpp"
#include "ifddir.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class OrfFile : public IfdFile {
    template<typename T>
    friend void audit_coefficients();

public:
    static RawFile *factory(const IO::Stream::Ptr &);
    OrfFile(const IO::Stream::Ptr &);
    virtual ~OrfFile();

    OrfFile(const OrfFile &) = delete;
    OrfFile &operator=(const OrfFile &) = delete;

protected:
    ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;
    virtual ::or_error _getRawData(RawData &data, uint32_t options) override;

private:
    ::or_error decompress(uint32_t x, uint32_t y, RawData& data);
    ::or_error addThumbnail(std::vector<uint32_t>& list, uint32_t offset, uint32_t len);
    static RawFile::TypeId _typeIdFromModel(const std::string &model);

    static const IfdFile::camera_ids_t s_def[];
};
}
}
