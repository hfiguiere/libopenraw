/*
 * libopenraw - tiffepfile.cpp
 *
 * Copyright (C) 2007-2008 Hubert Figuiere
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

#include <vector>

#include "tiffepfile.h"
#include "ifdfilecontainer.h"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

TiffEpFile::TiffEpFile(IO::Stream * s, Type _type)
    : IfdFile(s, _type)
{
}


IfdDir::Ref  TiffEpFile::_locateCfaIfd()
{
	const IfdDir::Ref & _mainIfd = mainIfd();

    std::vector<IfdDir::Ref> subdirs;
    if (!_mainIfd || !_mainIfd->getSubIFDs(subdirs)) {
        // error
        Trace(DEBUG1) << "couldn't find main ifd nor subifds\n";
        return IfdDir::Ref();
    }
    IfdDir::RefVec::const_iterator i = find_if(subdirs.begin(), 
                                               subdirs.end(),
                                               IfdDir::isPrimary());
    if (i != subdirs.end()) {
        return *i;
    }
    Trace(DEBUG1) << "couldn't find a primary subifd\n";
    return IfdDir::Ref();
}

IfdDir::Ref  TiffEpFile::_locateMainIfd()
{
    return m_container->setDirectory(0);
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
