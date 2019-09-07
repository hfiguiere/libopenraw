/*
 * libopenraw - mosaicinfo.h
 *
 * Copyright (C) 2012-2019 Hubert Figuière
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
#include <libopenraw/consts.h>

namespace OpenRaw {

class MosaicInfo
{
public:
  virtual ~MosaicInfo();

  /** Set the pattern size */
  void setSize(uint16_t x, uint16_t y);
  /** Get the pattern size */
  void getSize(uint16_t &x, uint16_t &y) const;

  /** Return of the mosaic is a Color Filter Array */
  bool isCFA() const;

  /** Return if the pattern is 2x2 RGB */
  bool is2by2Rgb() const;

  /**
   * @return the pattern type. Be cautious as this does not cover
   * non 2x2 RGB.
   */
  ::or_cfa_pattern patternType() const;
  const uint8_t* patternPattern(uint16_t& count) const;

  /** factory to return a singleton instance of the right pattern
   *  @return a const MosaicInfo. Never delete it. MAY BE NULL.
   */
  static const MosaicInfo* twoByTwoPattern(::or_cfa_pattern);

protected:
  MosaicInfo();
  MosaicInfo(::or_cfa_pattern pattern, uint16_t width, uint16_t height);

  /** Set the pattern pattern.
   * @param pattern the actual pattern sequence left to right,
   *  top to bottom
   * @param count the number of element. Should be width x height
   */
  void setPatternPattern(const uint8_t* pattern, uint16_t count);

  MosaicInfo(const MosaicInfo &) = delete;
  MosaicInfo& operator=(const MosaicInfo&) = delete;

  void setPatternType(::or_cfa_pattern pattern);

  class Private;

  Private *d;
};


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
