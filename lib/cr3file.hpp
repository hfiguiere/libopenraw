/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - cr3file.hpp
 *
 * Copyright (C) 2018-2020 Hubert Figuière
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

#include <array>
#include <memory>

#include <libopenraw/consts.h>

#include "io/stream.hpp"
#include "rawfile.hpp"
#include "ifdfilecontainer.hpp"
#include "makernotedir.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class IsoMediaContainer;

/** @addtogroup canon
 * @{
 */

/** @brief Canon CR3 file */
class Cr3File : public RawFile {
    template<typename T>
    friend void audit_coefficients();

public:
    static RawFile *factory(const IO::Stream::Ptr &s);
    Cr3File(const IO::Stream::Ptr &s);
    virtual ~Cr3File();

    Cr3File(const Cr3File &) = delete;
    Cr3File &operator=(const Cr3File &) = delete;

protected:
    virtual ::or_error _enumThumbnailSizes(
        std::vector<uint32_t> &list) override;
    virtual RawContainer* getContainer() const override;
    virtual ::or_error _getRawData(RawData &data, uint32_t options) override;

    virtual MetaValue* _getMetaValue(int32_t /*meta_index*/) override;

    virtual void _identifyId() override;

    virtual IfdDir::Ref _locateCfaIfd() override
        {
            LOGERR("not implemented\n");
            return IfdDir::Ref();
        }
    virtual IfdDir::Ref _locateMainIfd() override;
    virtual IfdDir::Ref _locateExifIfd() override;
    virtual MakerNoteDir::Ref _locateMakerNoteIfd() override;

private:
    IfdDir::Ref findIfd(uint32_t idx);

    IO::Stream::Ptr m_io; /**< the IO handle */
    IsoMediaContainer *m_container;
    std::array<std::shared_ptr<IfdFileContainer>, 4> m_ifds;

    static const RawFile::camera_ids_t s_def[];
};

/** @} */

}
}
