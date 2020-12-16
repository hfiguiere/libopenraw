/*
 * libopenraw - option.t.cpp
 *
 * Copyright (C) 2017-2020 Hubert Figuière
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

#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>

#include <string>

#include "option.hpp"

boost::unit_test::test_suite* init_unit_test_suite(int, char**)
{
  return nullptr;
}

BOOST_AUTO_TEST_CASE(test_option)
{
  Option<std::string> result;

  // Default, option is empty
  BOOST_CHECK(result.empty());
  bool unwrapped = false;
  try {
    result.value();
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
  BOOST_CHECK(result);
  BOOST_CHECK(result.value() == "hello world");
  BOOST_CHECK(result.empty());
  BOOST_CHECK(!result);
  // try unwrapping again
  unwrapped = false;
  try {
    result.value();
    unwrapped = true;
  } catch(std::runtime_error&) {
    BOOST_CHECK(true);
  } catch(...) {
    BOOST_CHECK(false);
  }
  BOOST_CHECK(!unwrapped);
  BOOST_CHECK(result.empty());
  BOOST_CHECK(result.value_or("good bye") == "good bye");
}

