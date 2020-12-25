/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - crwfile.hpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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
#include <sys/types.h>
#include <string>
#include <vector>

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "rawcontainer.hpp"
#include "io/stream.hpp"

namespace OpenRaw {

class MetaValue;
class RawData;

namespace Internals {

/** @addtogroup canon
 * @{
 */

namespace CIFF {
    class CiffMainIfd;
    class CiffExifIfd;
}

class CIFFContainer;

/** @brief Canon CRW file */
class CRWFile
    : public OpenRaw::RawFile
{
    template<typename T>
    friend void audit_coefficients();
    friend class CIFF::CiffMainIfd;
    friend class CIFF::CiffExifIfd;

public:
    static RawFile *factory(const IO::Stream::Ptr &);
    CRWFile(const IO::Stream::Ptr &);
    virtual ~CRWFile();

    CRWFile(const CRWFile&) = delete;
    CRWFile & operator=(const CRWFile&) = delete;

protected:


    virtual RawContainer* getContainer() const override;
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list) override;

    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
    virtual IfdDir::Ref _locateCfaIfd() override
        {
            return _locateMainIfd();
        }
    virtual IfdDir::Ref _locateMainIfd() override;
    virtual IfdDir::Ref _locateExifIfd() override;
    virtual MakerNoteDir::Ref _locateMakerNoteIfd() override
        {
            return MakerNoteDir::Ref();
        }
    virtual MetaValue *_getMetaValue(int32_t meta_index) override;

    virtual void _identifyId() override;

    Option<uint32_t> getOrientation() const;
    Option<std::string> getMakeOrModel(uint32_t index);

private:
    IO::Stream::Ptr m_io; /**< the IO handle */
    CIFFContainer *m_container; /**< the real container */
    uint32_t m_x;
    uint32_t m_y;
    std::string m_make;
    std::string m_model;

    static const RawFile::camera_ids_t s_def[];
};

/** @} */
}
}
