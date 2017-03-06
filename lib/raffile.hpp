/* -*- Mode: C++ ; tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - raffile.h
 *
 * Copyright (C) 2011-2017 Hubert Figuière
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

#ifndef OR_INTERNALS_RAFFILE_H_
#define OR_INTERNALS_RAFFILE_H_

#include <stdint.h>
#include <vector>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "rawcontainer.hpp"
#include "io/stream.hpp"

#define RAF_MAGIC "FUJIFILMCCD-RAW "
#define RAF_MAGIC_LEN 16

namespace OpenRaw {

class RawData;
class MetaValue;

namespace Internals {

class RafContainer;

class RafFile : public OpenRaw::RawFile {
public:
    static RawFile *factory(const IO::Stream::Ptr &s);
    RafFile(const IO::Stream::Ptr &s);
    virtual ~RafFile();

    RafFile(const RafFile &) = delete;
    RafFile &operator=(const RafFile &) = delete;

protected:
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;

    virtual RawContainer *getContainer() const override;

    virtual ::or_error _getRawData(RawData &data, uint32_t options) override;

    virtual MetaValue *_getMetaValue(int32_t /*meta_index*/) override;

    virtual void _identifyId() override;

private:
    bool isXTrans(RawFile::TypeId type) const;

    IO::Stream::Ptr m_io;      /**< the IO handle */
    RafContainer *m_container; /**< the real container */
    uint32_t m_x;
    uint32_t m_y;

    static const RawFile::camera_ids_t s_def[];
};
}
}

#endif
