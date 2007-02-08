/*
 * libopenraw - rawfilefactory.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <iostream>
#include <cassert>

#include "rawfilefactory.h"
#include "debug.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {


		RawFileFactory::RawFileFactory(RawFile::Type type, 
																	 RawFileFactory::raw_file_creator fn,
																	 const char *ext)
		{
			Trace(DEBUG1) << "registering type " 
										<< (int)type << "\n";
			registerType(type, fn, ext);
		}


		void RawFileFactory::registerType(RawFile::Type type, 
																			RawFileFactory::raw_file_creator fn,
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

	}
}

