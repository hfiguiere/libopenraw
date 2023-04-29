/*
 * libopenraw - rawfilefactory.cpp
 *
 * Copyright (C) 2006-2023 Hubert Figuière
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

#include <stddef.h>
#include <stdlib.h>

#include <cassert>
#include <utility>

#include <libopenraw/debug.h>

#include "rawfile.hpp"
#include "rawfilefactory.hpp"
#include "trace.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

/** accessor. This make sure the instance has been
 * constructed when needed
 */
RawFileFactory::Table &RawFileFactory::table_mut()
{
    /** the factory table */
    static Table rawFactoryTable;
    return rawFactoryTable;
}

RawFileFactory::Extensions &RawFileFactory::extensions_mut()
{
    /** the factory table */
    static Extensions rawExtensionsTable;
    return rawExtensionsTable;
}

void RawFileFactory::registerType(RawFile::Type type,
                                  const RawFileFactory::raw_file_factory_t &fn,
                                  const char *ext)
{
    if (fn == nullptr) {
        LOGERR("NULL fn for registerFactory()\n");
        assert(fn == nullptr);
    }
    table_mut()[type] = fn;
    extensions_mut()[ext] = type;
}

void RawFileFactory::unRegisterType(RawFile::Type type)
{
    Table::iterator iter = table_mut().find(type);
    if (iter == table().end()) {
        LOGERR("attempting to unregisterFactory() in unregistered element\n");
        assert(true);
    }
    table_mut().erase(iter);
}

const char **RawFileFactory::fileExtensions()
{
    static const char **_fileExtensions = NULL;
    if (!_fileExtensions) {
        const auto& ext = extensions();
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
