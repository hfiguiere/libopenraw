/*
 * libopenraw - thumbnail.cpp
 *
 * Copyright (C) 2005-2016 Hubert Figuiere
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

#include <stdint.h>
#include <cstdlib>
#include <memory>

#include <libopenraw/consts.h>

#include "bitmapdata.hpp"
#include "rawfile.hpp"
#include "thumbnail.hpp"

namespace OpenRaw {

/** Private data for the thumbnail class */
class Thumbnail::Private {
public:
    Private() {}

    ~Private() {}

    Private(const Private &) = delete;
    Private &operator=(const Private &) = delete;
};

Thumbnail::Thumbnail() : BitmapData(), d(new Thumbnail::Private())
{
}

Thumbnail::~Thumbnail()
{
    delete d;
}

Thumbnail *Thumbnail::getAndExtractThumbnail(const char *_filename,
                                             uint32_t preferred_size,
                                             or_error &err)
{
    err = OR_ERROR_NONE;
    Thumbnail *thumb = NULL;

    std::unique_ptr<RawFile> file(RawFile::newRawFile(_filename));
    if (file) {
        thumb = new Thumbnail();
        err = file->getThumbnail(preferred_size, *thumb);
    } else {
        err = OR_ERROR_CANT_OPEN; // file error
    }
    return thumb;
}
}
