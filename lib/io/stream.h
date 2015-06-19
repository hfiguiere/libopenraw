/*
 * libopenraw - stream.h
 *
 * Copyright (C) 2006-2015 Hubert Figuière
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


#ifndef OR_INTERNALS_IO_STREAM_H_
#define OR_INTERNALS_IO_STREAM_H_

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include <libopenraw/consts.h>


namespace OpenRaw {
namespace IO {
    
/** 
 * @brief base virtual class for IO
 */
class Stream
{
public:
  typedef std::shared_ptr<Stream> Ptr;
  /** Construct the file 
   * @param filename the full uri for the file
   */
  Stream(const char *filename);
  virtual ~Stream();
			
  /** Error type.
   * @see or_error
   */
  typedef ::or_error Error;
			
// file APIs
  /** open the file */
  virtual Error open() = 0;
  /** close the file */
  virtual int close() = 0;
  /** seek in the file. Semantics are similar to POSIX lseek() */
  virtual int seek(off_t offset, int whence) = 0;
  /** read in the file. Semantics are similar to POSIX read() */
  virtual int read(void *buf, size_t count) = 0;
  virtual off_t filesize() = 0;
//			virtual void *mmap(size_t l, off_t offset) = 0;
//			virtual int munmap(void *addr, size_t l) = 0;
			
  Error get_error()
    {
      return m_error;
    }

  /** get the uri path of the file */
  const std::string &get_path() const
    {
      return m_fileName;
    }

  uint8_t readByte() noexcept(false);
protected:
  void set_error(Error error)
    {
      m_error = error;
    }

private:
  /** private copy constructor to make sure it is not called */
  Stream(const Stream& f);
  /** private = operator to make sure it is never called */
  Stream & operator=(const Stream&);
			
  /** the file name (full path) */
  std::string m_fileName;
  Error m_error;
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
