/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
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
  typedef T data_type;

  Option()
    : m_none(true)
    , m_data()
  {
  }
  Option(T&& data)
    : m_none(false)
    , m_data(data)
  {
  }
  Option(const T& data)
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

  T&& unwrap()
  {
    if (m_none) {
      throw std::runtime_error("none option value");
    }
    m_none = true;
    return std::move(m_data);
  }
  T&& unwrap_or(T&& def)
  {
    if (m_none) {
      return std::move(def);
    }
    m_none = true;
    return std::move(m_data);
  }
  bool empty() const
  { return m_none; }
  bool ok() const
  { return !m_none; }
private:
  bool m_none;
  T m_data;
};
