/*
 * libopenraw - metavalue.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
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

using namespace Debug;

namespace OpenRaw {

MetaValue::MetaValue(const MetaValue & r)
    : m_value(r.m_value)
{
}

MetaValue::MetaValue(const value_t &v)
    : m_value(v)
{
}

namespace {

template <class T>
MetaValue::value_t convert(const Internals::IFDEntry::Ref & e)
{
    T v;
    v = Internals::IFDTypeTrait<T>::get(*e, 0, false);
    return MetaValue::value_t(v);
}

}

MetaValue::MetaValue(const Internals::IFDEntry::Ref & e)
{
    switch(e->type()) {
    case Internals::IFD::EXIF_FORMAT_BYTE:
    {
        m_value = convert<uint8_t>(e);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_ASCII:
    {
        m_value = convert<std::string>(e);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_SHORT:
    {
        m_value = convert<uint16_t>(e);
        break;
    }
    case Internals::IFD::EXIF_FORMAT_LONG:
    {
        m_value = convert<uint32_t>(e);
        break;
    }
    default:
        Trace(DEBUG1) << "unhandled type " << e->type() << "\n";
        break;
    }
}

template<typename T>
inline	T MetaValue::get() const
    throw(Internals::BadTypeException)
{
    T v;
    assert(!m_value.empty());
    try {
        v = boost::get<T>(m_value);
    }
    catch(...) { //const boost::bad_any_cast &) {
        throw Internals::BadTypeException();
    }
    return v;
}


uint32_t MetaValue::getInteger() const
    throw(Internals::BadTypeException)
{
    return get<uint32_t>();
}

std::string MetaValue::getString() const
    throw(Internals::BadTypeException)
{
    return get<std::string>();
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
