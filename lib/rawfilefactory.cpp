/*
 * libopenraw - rawfilefactory.cpp
 *
 * Copyright (C) 2006, 2008 Hubert Figuiere
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

#include <iostream>
#include <cassert>

#include "rawfilefactory.h"
#include "trace.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {


		RawFileFactory::RawFileFactory(RawFile::Type type, 
									   const RawFileFactory::raw_file_factory_t & fn,
									   const char *ext)
		{
			Trace(DEBUG1) << "registering type " 
						  << (int)type << "\n";
			registerType(type, fn, ext);
		}


		void RawFileFactory::registerType(RawFile::Type type, 
										  const RawFileFactory::raw_file_factory_t & fn,
										  const char *ext)
		{
			if (fn == NULL)
			{
				Trace(ERROR) << "NULL fn for registerFactory()\n";
				assert(fn == NULL);
			}
			table()[type] = fn;
			extensions()[ext] = type;
		}


		void RawFileFactory::unRegisterType(RawFile::Type type)
		{
			Table::iterator iter = table().find(type);
			if (iter == table().end())
			{
				Trace(ERROR) << "attempting to unregisterFactory() in unregistered "
					"element\n";
				assert(true);
			}
			table().erase(iter);
		}

const char **RawFileFactory::fileExtensions()
{
    static const char **_fileExtensions = NULL;
    if(!_fileExtensions) {
        Extensions & ext = extensions();
        size_t s = ext.size();
        _fileExtensions = (const char**)calloc((s + 1), sizeof(char*));
        const char **current = _fileExtensions;
        Extensions::const_iterator iter(ext.begin());
        for ( ; iter != ext.end(); ++iter) {
            *current = iter->first.c_str();
            current++;
        }
    }

    return _fileExtensions;
}

	}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
