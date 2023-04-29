/*
 * libopenraw - rawfilefactory.hpp
 *
 * Copyright (C) 2006-2023 Hubert Figuiere
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

#include <functional>
#include <map>
#include <string>

#include "io/stream.hpp"
#include "rawfile.hpp"

namespace OpenRaw {
namespace Internals {

class RawFileFactory {
public:
    typedef std::function<RawFile *(const IO::Stream::Ptr &)>
        raw_file_factory_t;
    /** the factory type for raw files
     * key is the extension. file is factory method
     */
    typedef std::map<RawFile::Type, raw_file_factory_t> Table;
    typedef std::map<std::string, RawFile::Type> Extensions;

    RawFileFactory() = delete;
    ~RawFileFactory() = delete;

    /** Access the table. Ensure that it has been constructed. */
    static const Table &table() {
        return table_mut();
    }
    /** Access the extensions table. Ensure that it has been constructed. */
    static const Extensions &extensions() {
        return extensions_mut();
    }

    /** access the the list of file extenstions registered. */
    static const char **fileExtensions();

    /** register a filetype with the factory
     * @param type the type of file
     * @param fn the factory method
     * @param ext the extension associated
     * @note it is safe to call this method with the same
     * fn and type to register a different extension
     */
    static void registerType(RawFile::Type type, const raw_file_factory_t &fn,
                             const char *ext);
private:
    /** Access the table mutably. Ensure that it has been constructed. */
    static Table &table_mut();
    /** Access the extensions table mutably. Ensure that it has been constructed. */
    static Extensions &extensions_mut();
    /** Unregister type from the table. */
    static void unRegisterType(RawFile::Type type);
};

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
