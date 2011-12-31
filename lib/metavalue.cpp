/*
 * libopenraw - metavalue.cpp
 *
 * Copyright (C) 2007,2011-2012 Hubert Figuiere
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


#include "trace.h"
#include "exception.h"
#include "metavalue.h"
#include "ifdentry.h"

using namespace Debug;

namespace OpenRaw {

MetaValue::MetaValue(const MetaValue & r)
    : m_values(r.m_values)
{
}

MetaValue::MetaValue(const value_t &v)
{
    m_values.push_back(v);
}

MetaValue::MetaValue(const std::vector<value_t> &v)
    : m_values(v)
{
    
}

namespace {

template <class T>
void convert(const Internals::IfdEntry::Ref & e, std::vector<MetaValue::value_t> & values)
{
    std::vector<T> v;
    e->getArray(v);
    values.insert(values.end(), v.begin(), v.end());
}

// T is the Ifd primitive type. T2 is the target MetaValue type.
template <class T, class T2>
void convert(const Internals::IfdEntry::Ref & e, std::vector<MetaValue::value_t> & values)
{
    std::vector<T> v;
    e->getArray(v);
    for(typename std::vector<T>::const_iterator iter = v.begin(); iter != v.end(); ++iter) {
        values.push_back(T2(*iter));
    }
}

}

MetaValue::MetaValue(const Internals::IfdEntry::Ref & e)
{
    switch(e->type()) {
    case Internals::IFD::EXIF_FORMAT_BYTE:
    {
        convert<uint8_t, uint32_t>(e, m_values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_ASCII:
    {
        convert<std::string>(e, m_values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SHORT:
    {
        convert<uint16_t, uint32_t>(e, m_values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_LONG:
    {
        convert<uint32_t>(e, m_values);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SRATIONAL:
    {
        convert<Internals::IFD::SRational, double>(e, m_values);
        break;
    }
    default:
        Trace(DEBUG1) << "unhandled type " << e->type() << "\n";
        return;
    }
}

template<typename T>
inline	T MetaValue::get(int idx) const
    throw(Internals::BadTypeException)
{
    T v;
    assert(!m_values.empty());
    try {
        v = boost::get<T>(m_values[idx]);
    }
    catch(...) { //const boost::bad_any_cast &) {
        throw Internals::BadTypeException();
    }
    return v;
}


uint32_t MetaValue::getInteger(int idx) const
    throw(Internals::BadTypeException)
{
    return get<uint32_t>(idx);
}

std::string MetaValue::getString(int idx) const
    throw(Internals::BadTypeException)
{
    return get<std::string>(idx);
}

double MetaValue::getDouble(int idx) const
    throw(Internals::BadTypeException)
{
    return get<double>(idx);
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
