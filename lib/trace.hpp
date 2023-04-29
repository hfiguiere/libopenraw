/*
 * libopenraw - trace.hpp
 *
 * Copyright (C) 2006-2023 Hubert Figuière
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

#include <stdint.h>

#include <string>
#include <vector>
#include <algorithm>

#include <libopenraw/debug.h>

// use this to cast size_t to a format %lu
#define LSIZE long unsigned int
// use this to cast off_t to a format %lld
#define LOFFSET long long int

namespace OpenRaw {
namespace Internals {
class IfdDir;
}
}

namespace Debug {

/** Log debug messages. printf format.
 * @param fmt the formt string, printf style
 * @param func the func name
 */
void log(debug_level level, const char* fmt, ...)
    __attribute__ ((format (printf, 2, 3)));


#define LOGASSERT(x) \
  if (!(x)) Debug::log(ERROR, "ASSERT failed: %s\n", #x)

#define LOGWARN(...) \
  Debug::log(WARNING, ## __VA_ARGS__)

#define LOGERR(...) \
  Debug::log(ERROR, ## __VA_ARGS__)

#define LOGDBG1(...) \
  Debug::log(DEBUG1, ## __VA_ARGS__)

#define LOGDBG2(...) \
  Debug::log(DEBUG2, ## __VA_ARGS__)

/** Convert bytes to a displayed string for Debug */
std::string bytes_to_string(const uint8_t* bytes, size_t len);
/** Convert ascii bytes to a displayed string for Debug */
std::string ascii_to_string(const uint8_t* bytes, size_t len);
/** Convert dump an IFD to a string */
std::string dump_ifd(const OpenRaw::Internals::IfdDir& dir);

/** a basic Trace class for debug */
class Trace
{
public:
  Trace(debug_level level)
    : m_level(level)
    {
    }
  Trace & operator<<(int i);
  Trace & operator<<(const char * s);
  Trace & operator<<(void *);
  Trace & operator<<(const std::string & s);

  template <class T>
  Trace & operator<<(const std::vector<T> & v);

  static void setDebugLevel(debug_level lvl);
private:
  friend void log(debug_level level, const char* fmt, ...);
  static void print(int i);
  static int debugLevel; // global debug level
  int m_level;
};


template <class T>
Trace & Trace::operator<<(const std::vector<T> & v)
{
  if (m_level <= debugLevel) {
    std::for_each(v.cbegin(), v.cend(),
                  [](T a) {
                    print(a);
                  });
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
