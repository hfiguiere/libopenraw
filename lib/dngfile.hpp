/* -*- Mode: C++ -*- */
/*
 * libopenraw - dngfile.hpp
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

#include <libopenraw/consts.h>

#include "rawfile.hpp"
#include "ifdfile.hpp"
#include "io/stream.hpp"
#include "tiffepfile.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

/** @defgroup dng_parsing DNG parsing
 * @ingroup ifd_parsing
 *
 * @brief DNG file parsing.
 *
 * @{
 */

/** @brief DNG file */
class DngFile
    : public TiffEpFile
{
    template<typename T>
    friend void audit_coefficients();
public:
    static RawFile *factory(const IO::Stream::Ptr &);

    DngFile(const IO::Stream::Ptr &);
    virtual ~DngFile();

    DngFile(const DngFile&) = delete;
    DngFile & operator=(const DngFile&) = delete;


    /** @inherit */
    virtual or_colour_matrix_origin getColourMatrixOrigin() const override;

    /** @brief Tell if a file is Cinema DNG.
     *
     * DNG specific for now.
     */
    bool isCinema() const;
protected:
    /** @inherit */
    virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t>& list) override;
    /** @inherit */
    virtual ::or_error _getRawData(RawData & data, uint32_t options) override;
    /** @inherit */
    virtual void _identifyId() override;

private:

    static const IfdFile::camera_ids_t s_def[];
};

}
}
