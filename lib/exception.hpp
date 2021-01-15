/*
 * libopenraw - exception.hpp
 *
 * Copyright (C) 2006-2021 Hubert Figuière
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

#include <exception>
#include <string>
#include <typeinfo>

namespace OpenRaw {
namespace Internals {

/** @addtogroup internals
 * @{ */

/** @brief Generic OpenRaw exception */
class Exception
  : public std::exception
{
protected:
  /** @brief Exception error string */
  std::string m_what;
public:
  Exception()
    : std::exception(),
      m_what()
    {}
  /** @brief Construct an exception with a message strings */
  Exception(const std::string &w)
    : std::exception(),
      m_what(w)
    {}
  virtual ~Exception()
    {}
  /** @brief the std::exception::what() override */
  const char *what() const noexcept(true) override
    {
      if(m_what.empty()) {
        return typeid(this).name();
      }
      return m_what.c_str();
    }
};

/** @brief IO exception */
class IOException
  : public Exception
{
public:
  IOException(const std::string &w)
    : Exception(w)
    {}
  /** @inherit */
  const char *what() const noexcept(true) override
    {
      if(m_what.empty()) {
        return typeid(this).name();
      }
      return m_what.c_str();
    }
};


/** @brief Data is of bad type */
class BadTypeException
  : public Exception
{
public:
  /** @inherit */
  const char *what() const noexcept(true) override
    {
      if(m_what.empty()) {
        return typeid(this).name();
      }
      return m_what.c_str();
    }
};

/** @brief Data is of too big */
class TooBigException
  : public Exception
{
public:
  /** @inherit */
  const char *what() const noexcept(true) override
    {
      if(m_what.empty()) {
        return typeid(this).name();
      }
      return m_what.c_str();
    }
};

/** @brief Out of range index */
class OutOfRangeException
  : public Exception
{
public:
  /** @inherit */
  const char *what() const noexcept(true) override
    {
      if(m_what.empty()) {
        return typeid(this).name();
      }
      return m_what.c_str();
    }
};

/** @brief Decoding error */
class DecodingException
  : public Exception
{
public:
  DecodingException(const std::string &w)
    : Exception(w)
    {}
  /** @inherit */
  const char *what() const noexcept(true) override
    {
      if(m_what.empty()) {
        return typeid(this).name();
      }
      return m_what.c_str();
    }
};

/** @} */
}
}

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
