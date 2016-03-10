/*
 * libopenraw - trace.h
 *
 * Copyright (C) 2006-2015 Hubert Figuiere
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


#ifndef OR_INTERNALS_TRACE_H_
#define OR_INTERNALS_TRACE_H_

#include <string>
#include <vector>
#include <algorithm>

#include <libopenraw/debug.h>

namespace Debug {

/** Log debug messages. printf format.
 * @param fmt the formt string, printf style
 * @param func the func name
 */
void log(debug_level level, const char* fmt, ...)
    __attribute__ ((format (printf, 2, 3)));


#define LOGWARN(...) \
  Debug::log(WARNING, ## __VA_ARGS__)

#define LOGERR(...) \
  Debug::log(ERROR, ## __VA_ARGS__)

#define LOGDBG1(...) \
  Debug::log(DEBUG1, ## __VA_ARGS__)

#define LOGDBG2(...) \
  Debug::log(DEBUG2, ## __VA_ARGS__)

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

#endif
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
