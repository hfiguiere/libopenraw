/* -*- mode: C++; tab-width: 2; c-basic-offset: 2; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - option.hpp
 *
 * Copyright (C) 2017 Hubert Figuière
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

// an option<> template class inspired by Rust

#pragma once

#include <stdexcept>

template<class T>
class Option
{
public:
  typedef T value_type;

  Option()
    : m_none(true)
    , m_data()
  {
  }
  explicit Option(T&& data)
    : m_none(false)
    , m_data(data)
  {
  }
  explicit Option(const T& data)
    : m_none(false)
    , m_data(data)
  {
  }
  template<class... Args>
  Option(Args&&... args)
    : m_none(false)
    , m_data(args...)
  {
  }

  T&& value()
  {
    if (m_none) {
      throw std::runtime_error("none option value");
    }
    m_none = true;
    return std::move(m_data);
  }
  T&& value_or(T&& def)
  {
    if (m_none) {
      return std::move(def);
    }
    m_none = true;
    return std::move(m_data);
  }

  bool empty() const
  { return m_none; }

  constexpr explicit operator bool() const
  { return !m_none; }
  constexpr bool has_value() const
  { return !m_none; }
private:
  bool m_none;
  T m_data;
};
