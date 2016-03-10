/*
 * libopenraw - cfapattern.cpp
 *
 * Copyright (C) 2012 Hubert Figuière
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

#include <stdint.h>
#include <stddef.h>

#include <array>

#include <boost/static_assert.hpp>

#include <libopenraw/consts.h>

#include "cfapattern.hpp"

namespace OpenRaw {

namespace Internals {

/** alias the colours. */
static const uint8_t RED = OR_PATTERN_COLOUR_RED;
static const uint8_t GREEN = OR_PATTERN_COLOUR_GREEN;
static const uint8_t BLUE = OR_PATTERN_COLOUR_BLUE;

static const uint8_t RGGB_PATTERN[] = { RED, GREEN, GREEN, BLUE };
static const uint8_t GBRG_PATTERN[] = { GREEN, BLUE, RED, GREEN };
static const uint8_t BGGR_PATTERN[] = { BLUE, GREEN, GREEN, RED };
static const uint8_t GRBG_PATTERN[] = { GREEN, RED, BLUE, GREEN };

class Cfa2x2RgbPattern
  : public CfaPattern
{
public:
  Cfa2x2RgbPattern(::or_cfa_pattern pattern)
    : CfaPattern(pattern, 2, 2)
    {
      switch(pattern) {
      case OR_CFA_PATTERN_RGGB:
        setPatternPattern(RGGB_PATTERN, 4);
        break;
      case OR_CFA_PATTERN_GBRG:
        setPatternPattern(GBRG_PATTERN, 4);
        break;
      case OR_CFA_PATTERN_BGGR:
        setPatternPattern(BGGR_PATTERN, 4);
        break;
      case OR_CFA_PATTERN_GRBG:
        setPatternPattern(GRBG_PATTERN, 4);
        break;

      default:
        break;
      }
    }

};

}

const CfaPattern*
CfaPattern::twoByTwoPattern(::or_cfa_pattern pattern)
{
  static std::array<CfaPattern*, _OR_CFA_PATTERN_INVALID> s_patterns
    = { { NULL, NULL, NULL, NULL, NULL, NULL } };
  // this should be updated if we change the enum
  BOOST_STATIC_ASSERT(_OR_CFA_PATTERN_INVALID == 6);

  if((pattern == OR_CFA_PATTERN_NON_RGB22) ||
     (pattern >= _OR_CFA_PATTERN_INVALID)) {
    return NULL;
  }

  CfaPattern* pat = s_patterns[pattern];
  if(!pat) {
    pat = new Internals::Cfa2x2RgbPattern(pattern);
    s_patterns[pattern] = pat;
  }

  return pat;
}


class CfaPattern::Private
{
public:
  friend class Internals::Cfa2x2RgbPattern;

  Private()
    : x(0), y(0), n_colours(0)
    , pattern_type(OR_CFA_PATTERN_NONE)
    , pattern(NULL)
    {}

  uint16_t x;
  uint16_t y;
  uint16_t n_colours;
  ::or_cfa_pattern pattern_type;
  const uint8_t* pattern;
};

CfaPattern::CfaPattern()
  : d(new CfaPattern::Private)
{
}

CfaPattern::CfaPattern(::or_cfa_pattern pattern,
                       uint16_t width, uint16_t height)
  : d(new CfaPattern::Private)
{
  setSize(width, height);
  setPatternType(pattern);
}

CfaPattern::~CfaPattern()
{
  delete d;
}

void CfaPattern::setSize(uint16_t x, uint16_t y)
{
  d->x = x;
  d->y = y;
  if(x != 2 || y != 2) {
    d->pattern_type = OR_CFA_PATTERN_NON_RGB22;
  }
  else if(!is2by2Rgb()) {
    d->pattern_type = OR_CFA_PATTERN_NONE;
  }
}

bool CfaPattern::is2by2Rgb() const
{
  return (d->pattern_type != OR_CFA_PATTERN_NONE)
    && (d->pattern_type != OR_CFA_PATTERN_NON_RGB22);
}

void
CfaPattern::setPatternPattern(const uint8_t* pattern, uint16_t count)
{
  if(count != d->x * d->y) {
    d->pattern = NULL;
    // TODO deal with the error
    return;
  }
  d->pattern = pattern;
}

const uint8_t*
CfaPattern::patternPattern(uint16_t& count) const
{
  if(d->pattern) {
    count = d->x * d->y;
    return d->pattern;
  }
  
  count = 0;
  return NULL;
}

void CfaPattern::setPatternType(::or_cfa_pattern pattern)
{
  d->pattern_type = pattern;
  if(is2by2Rgb()) {
    setSize(2, 2);
    d->n_colours = 3;
  }
}

::or_cfa_pattern
CfaPattern::patternType() const
{
  return d->pattern_type;
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
