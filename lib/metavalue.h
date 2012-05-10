/*
 * libopenraw - metavalue.h
 *
 * Copyright (C) 2007, 2011-2012 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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


#ifndef _OPENRAW_METAVALUE_H
#define _OPENRAW_METAVALUE_H

#include <boost/variant.hpp>
#include <boost/array.hpp>
#include <libopenraw/types.h>

#include "ifdentry.h"

namespace OpenRaw {
	
class MetaValue
{
public:
    typedef boost::variant<std::string, uint32_t, double> value_t;

    MetaValue(const MetaValue &);
    template <class T> MetaValue(const T &v)
        {
            m_values.push_back(v);
        }
    template <class T> MetaValue(const std::vector<T> &v)
        : m_values(v)
        {
        }
    explicit MetaValue(const value_t &v);
    explicit MetaValue(const std::vector<value_t> &v);
    explicit MetaValue(const Internals::IfdEntry::Ref & e);

    uint32_t getCount() const
        {
            return m_values.size();
        }

    uint32_t getInteger(int idx) const
        throw(Internals::BadTypeException);
    std::string getString(int idx) const
        throw(Internals::BadTypeException);
    double getDouble(int idx) const
        throw(Internals::BadTypeException);
private:
    template<typename T> T get(int idx) const
        throw(Internals::BadTypeException);

    std::vector<value_t> m_values;
};


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

#endif
