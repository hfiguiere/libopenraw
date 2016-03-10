/*
 * libopenraw - rawfilefactory.h
 *
 * Copyright (C) 2006-2016 Hubert Figuiere
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

#ifndef OR_INTERNALS_RAWFILEFACTORY_H_
#define OR_INTERNALS_RAWFILEFACTORY_H_

#include <string>
#include <map>
#include <functional>

#include "rawfile.hpp"
#include "io/stream.hpp"

namespace OpenRaw {
namespace Internals {

class RawFileFactory
{
public:
    typedef std::function<RawFile*(const IO::Stream::Ptr&)> raw_file_factory_t;
    /** the factory type for raw files
     * key is the extension. file is factory method
     */
    typedef
    std::map<RawFile::Type, raw_file_factory_t> Table;
    typedef
    std::map<std::string, RawFile::Type> Extensions;

    /** register a filetype with the factory
     * @param type the type of file
     * @param fn the factory method
     * @param ext the extension associated
     * @note it is safe to call this method with the same
     * fn and type to register a different extension
     */
    RawFileFactory(RawFile::Type type,
                   const raw_file_factory_t & fn,
                   const char * ext);

    /** access the table. Ensure that it has been constructed. */
    static Table & table();
    /** access the extensions table. Ensure that it has been constructed. */
    static Extensions & extensions();

    /** access the the list of file extenstions registered. */
    static const char **fileExtensions();

    static void registerType(RawFile::Type type,
                             const raw_file_factory_t & fn,
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
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
