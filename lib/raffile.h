/*
 * libopenraw - raffile.h
 *
 * Copyright (C) 2011-2012 Hubert Figuiere
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

#ifndef __OPENRAW_RAFFILE_H_
#define __OPENRAW_RAFFILE_H_

#include <map>

#include <libopenraw++/rawfile.h>

#define RAF_MAGIC "FUJIFILMCCD-RAW "
#define RAF_MAGIC_LEN 16

namespace OpenRaw {
namespace Internals {
	
class RafContainer;
class ThumbDesc;

class RafFile
	: public OpenRaw::RawFile
{
public:
	static RawFile *factory(IO::Stream * s);
	RafFile(IO::Stream *s);
	virtual ~RafFile();
	
protected:
  virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list);
	
  virtual RawContainer* getContainer() const;

  virtual ::or_error _getRawData(RawData & data, uint32_t options);
	
  virtual MetaValue *_getMetaValue(int32_t /*meta_index*/);

  virtual void _identifyId();

private:
	RafFile(const RafFile&);
	RafFile & operator=(const RafFile&);
	
	IO::Stream *m_io; /**< the IO handle */
	RafContainer *m_container; /**< the real container */
	uint32_t m_x;
	uint32_t m_y;

	static const RawFile::camera_ids_t s_def[];	
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
