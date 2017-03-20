/*
 * libopenraw - rawfilefactory.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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

#include <stdlib.h>
#include <stddef.h>

#include <utility>
#include <cassert>

#include <libopenraw/debug.h>

#include "rawfile.hpp"
#include "rawfilefactory.hpp"
#include "trace.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

RawFileFactory::RawFileFactory(RawFile::Type type,
                               const RawFileFactory::raw_file_factory_t &fn,
                               const char *ext)
{
    LOGDBG1("registering type %d\n", (int)type);
    registerType(type, fn, ext);
}

void RawFileFactory::registerType(RawFile::Type type,
                                  const RawFileFactory::raw_file_factory_t &fn,
                                  const char *ext)
{
    if (fn == nullptr) {
        LOGERR("NULL fn for registerFactory()\n");
        assert(fn == nullptr);
    }
    table()[type] = fn;
    extensions()[ext] = type;
}

void RawFileFactory::unRegisterType(RawFile::Type type)
{
    Table::iterator iter = table().find(type);
    if (iter == table().end()) {
        LOGERR("attempting to unregisterFactory() in unregistered element\n");
        assert(true);
    }
    table().erase(iter);
}

const char **RawFileFactory::fileExtensions()
{
    static const char **_fileExtensions = NULL;
    if (!_fileExtensions) {
        Extensions &ext = extensions();
        size_t s = ext.size();
        _fileExtensions = (const char **)calloc((s + 1), sizeof(char *));
        const char **current = _fileExtensions;
        Extensions::const_iterator iter(ext.begin());
        for (; iter != ext.end(); ++iter) {
            *current = iter->first.c_str();
            current++;
        }
    }

    return _fileExtensions;
}
}
}

