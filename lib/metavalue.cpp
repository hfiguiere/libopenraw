/*
 * libopenraw - metavalue.cpp
 *
 * Copyright (C) 2007-2020 Hubert Figuiere
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


#include <assert.h>

#include "metavalue.hpp"
#include "exception.hpp"

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

template<typename T>
inline T MetaValue::get(int idx) const noexcept(false)
{
    assert(!m_values.empty());
    try {
        return boost::get<T>(m_values[idx]);
    }
    catch(...) { //const boost::bad_any_cast &) {
        throw Internals::BadTypeException();
    }
}

template<typename T>
inline const T & MetaValue::getRef(int idx) const noexcept(false)
{
    static const T v;
    assert(!m_values.empty());
    try {
        return boost::get<T>(m_values[idx]);
    }
    catch(...) { //const boost::bad_any_cast &) {
        throw Internals::BadTypeException();
    }
    return v;
}

uint32_t MetaValue::getUInteger(int idx) const
{
    return get<uint32_t>(idx);
}

int32_t MetaValue::getSInteger(int idx) const
{
    return get<int32_t>(idx);
}

const std::string & MetaValue::getString(int idx) const
{
    return getRef<std::string>(idx);
}

double MetaValue::getDouble(int idx) const
{
    return get<double>(idx);
}

/// Formatter for getAsString()
class to_string_visitor
    : public boost::static_visitor<std::string>
{
public:
    std::string operator()(const std::string& s) {
        return s;
    }
    std::string operator()(const uint8_t v) {
        return std::to_string(v);
    }
    std::string operator()(uint32_t v) {
        return std::to_string(v);
    }
    std::string operator()(int32_t v) {
        return std::to_string(v);
    }
    std::string operator()(double v) {
        return std::to_string(v);
    }
};

const std::string & MetaValue::getAsString() const
{
    if (m_as_str.empty()) {
        auto visitor = to_string_visitor();
        bool multiple = (getCount() > 1);
        if (multiple) {
            m_as_str += "[ ";
        }
        for (auto value : m_values) {
            m_as_str += value.apply_visitor(visitor);
            if (multiple) {
                m_as_str += ", ";
            }
        }
        if (multiple) {
            m_as_str += "]";
        }
    }
    return m_as_str;
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
