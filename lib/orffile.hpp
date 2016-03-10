/* -*- Mode: C++ -*- */
/*
 * libopenraw - orffile.h
 *
 * Copyright (C) 2006-2016 Hubert Figuière
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

#ifndef OR_INTERNALS_ORFFILE_H_
#define OR_INTERNALS_ORFFILE_H_

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
public:
    static RawFile *factory(const IO::Stream::Ptr &);
    OrfFile(const IO::Stream::Ptr &);
    virtual ~OrfFile();

    OrfFile(const OrfFile &) = delete;
    OrfFile &operator=(const OrfFile &) = delete;

    enum { ORF_COMPRESSION = 0x10000 };

protected:
    virtual IfdDir::Ref _locateCfaIfd() override;
    virtual IfdDir::Ref _locateMainIfd() override;

    ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;
    virtual ::or_error _getRawData(RawData &data, uint32_t options) override;
    virtual uint32_t _translateCompressionType(
        IFD::TiffCompress tiffCompression) override;

private:
    static RawFile::TypeId _typeIdFromModel(const std::string &model);

    static const IfdFile::camera_ids_t s_def[];
};
}
}

#endif
