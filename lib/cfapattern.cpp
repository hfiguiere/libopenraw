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
#include <vector>

#include <libopenraw/consts.h>
#include <libopenraw++/cfapattern.h>

namespace OpenRaw {

namespace Internals {

class Cfa2x2RgbPattern
  : public CfaPattern
{
public:
  Cfa2x2RgbPattern(::or_cfa_pattern pattern)
    : CfaPattern(pattern)
    {
    }

};

}

const CfaPattern* 
CfaPattern::twoByTwoPattern(::or_cfa_pattern pattern)
{
  static std::vector<CfaPattern*> s_patterns(_OR_CFA_PATTERN_INVALID, NULL);

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
    : x(0), y(0), n_colors(0)
    , pattern_type(OR_CFA_PATTERN_NONE)
    {}

  uint16_t x;
  uint16_t y;
  uint16_t n_colors;
  ::or_cfa_pattern pattern_type;
};

CfaPattern::CfaPattern()
  : d(new CfaPattern::Private)
{
}

CfaPattern::CfaPattern(::or_cfa_pattern pattern)
  : d(new CfaPattern::Private)
{
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

void CfaPattern::setPatternType(::or_cfa_pattern pattern)
{
  d->pattern_type = pattern;
  if(is2by2Rgb()) {
    setSize(2, 2);
    d->n_colors = 3;
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
