/*
 * libopenraw - trace.cpp
 *
 * Copyright (C) 2006-2014 Hubert Figuiere
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

#include <stdarg.h>
#include <stdio.h>

#include <iostream>

#include <libopenraw/debug.h>

#include "trace.hpp"

namespace Debug {

int Trace::debugLevel = NOTICE;

void log(debug_level level, const char *fmt, ...)
{
  if (level > Trace::debugLevel) {
    return;
  }

  va_list marker;

  va_start(marker, fmt);
  vfprintf(stderr, fmt, marker);

  va_end(marker);
}

void Trace::setDebugLevel(debug_level lvl)
{
  debugLevel = lvl;
}

void Trace::print(int i)
{
  std::cerr << i << " ";
}

Trace & Trace::operator<<(int i)
{
  if (m_level <= debugLevel) {
    std::cerr << i;
  }
  return *this;
}

Trace & Trace::operator<<(const char * s)
{
  if (m_level <= debugLevel) {
    std::cerr << s;
  }
  return *this;
}

Trace & Trace::operator<<(void *p)
{
  if (m_level <= debugLevel) {
    std::cerr << p;
  }
  return *this;
}

Trace & Trace::operator<<(const std::string & s)
{
  if (m_level <= debugLevel) {
    std::cerr << s;
  }
  return *this;
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
