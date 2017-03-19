/*
 * libopenraw - option.t.cpp
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
/** @brief unit test for option */

#include <boost/test/minimal.hpp>

#include <stdlib.h>

#include <string>

#include "option.hpp"

int test_main( int, char *[] )             // note the name!
{
  Option<std::string> result;

  // Default, option is empty
  BOOST_CHECK(result.empty());
  bool unwrapped = false;
  try {
    result.unwrap();
    unwrapped = true;
  } catch(std::runtime_error&) {
    BOOST_CHECK(true);
  } catch(...) {
    BOOST_CHECK(false);
  }
  BOOST_CHECK(!unwrapped);
  BOOST_CHECK(result.empty());

  // Option with value
  result = Option<std::string>("hello world");
  BOOST_CHECK(!result.empty());
  BOOST_CHECK(result.ok());
  BOOST_CHECK(result.unwrap() == "hello world");
  BOOST_CHECK(result.empty());
  BOOST_CHECK(!result.ok());
  // try unwrapping again
  unwrapped = false;
  try {
    result.unwrap();
    unwrapped = true;
  } catch(std::runtime_error&) {
    BOOST_CHECK(true);
  } catch(...) {
    BOOST_CHECK(false);
  }
  BOOST_CHECK(!unwrapped);
  BOOST_CHECK(result.empty());
  BOOST_CHECK(result.unwrap_or("good bye") == "good bye");

  return 0;
}

