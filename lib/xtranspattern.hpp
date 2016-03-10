/*
 * libopenraw - xtranspattern.h
 *
 * Copyright (C) 2012 Hubert Figuière
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

#ifndef OR_INTERNALS_XTRANSPATTERN_H_
#define OR_INTERNALS_XTRANSPATTERN_H_

#include "cfapattern.hpp"

namespace OpenRaw {
namespace Internals {

/**
 * The X-Trans CMOS 6x6 pattern for the Fuji X-Pro1.
 */
class XTransPattern
	: public CfaPattern
{
public:
  static const XTransPattern* xtransPattern();

protected:
  XTransPattern();
};

}
}
#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
