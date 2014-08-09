/*
 * libopenraw - orffile.h
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

#ifndef OR_INTERNALS_ORFFILE_H_
#define OR_INTERNALS_ORFFILE_H_

#include "ifdfile.h"

namespace OpenRaw {

class Thumbnail;
class RawData;

namespace Internals {

class OrfFile
  : public IfdFile
{
public:
  static RawFile *factory(const IO::Stream::Ptr &);
  OrfFile(const IO::Stream::Ptr &);
  virtual ~OrfFile();

  enum {
    ORF_COMPRESSION = 0x10000
  };

protected:
  virtual IfdDir::Ref  _locateCfaIfd();
  virtual IfdDir::Ref  _locateMainIfd();

  ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list);
  virtual ::or_error _getRawData(RawData & data, uint32_t options);
  virtual uint32_t _translateCompressionType(IFD::TiffCompress tiffCompression);
private:
  static RawFile::TypeId _typeIdFromModel(const std::string & model);

  OrfFile(const OrfFile&) = delete;
  OrfFile & operator=(const OrfFile &) = delete;

  static const IfdFile::camera_ids_t s_def[];
};

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
