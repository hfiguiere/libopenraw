/* -*- tab-width:2; c-basic-offset:2; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - isomediacontainer.hpp
 *
 * Copyright (C) 2018 Hubert Figuiere
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

#include "io/stream.hpp"
#include "rawcontainer.hpp"

namespace OpenRaw {
namespace Internals {

class IsoMediaContainer : public RawContainer {
public:
    IsoMediaContainer(const IO::Stream::Ptr &file);
};
}
}
