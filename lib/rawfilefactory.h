/*
 * libopenraw - rawfilefactory.h
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



#ifndef __RAWFILEFACTORY_H
#define __RAWFILEFACTORY_H


#include <string>
#include <map>

#include <libopenraw++/rawfile.h>

namespace OpenRaw {

	namespace Internals {

		
		class RawFileFactory
		{
		public:
			typedef RawFile * (*raw_file_creator)(const char *);
			/** the factory type for raw files
			 * key is the extension. file is factory method
			 */
			typedef 
			std::map<RawFile::Type, raw_file_creator> Table;
			typedef
			std::map<std::string, RawFile::Type> Extensions;
			
			/** register a filetype with the factory
			 * @param type the type of file
			 * @param fn the factory method
			 * @param ext the extension associated
			 * @note it is safe to call this method with the same
			 * fn and type to register a different extension
			 */
			RawFileFactory(RawFile::Type type, raw_file_creator fn, 
										 const char * ext);

			/** access the table. Ensure that it has been constructed. */
			static Table & table();
			/** access the extensions table. Ensure that it has been constructed. */
			static Extensions & extensions();

			static void registerType(RawFile::Type type, raw_file_creator fn,
															 const char * ext);
			static void unRegisterType(RawFile::Type type);
		};



		/** accessor. This make sure the instance has been
		 * constructed when needed
		 */
		inline RawFileFactory::Table & RawFileFactory::table()
		{
			/** the factory table */
			static Table rawFactoryTable;
			return rawFactoryTable;
		}

		inline RawFileFactory::Extensions & RawFileFactory::extensions()
		{
			/** the factory table */
			static Extensions rawExtensionsTable;
			return rawExtensionsTable;
		}
		
	}
}

#endif
