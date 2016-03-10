/*
 * libopenraw - ifdfilecontainer.h
 *
 * Copyright (C) 2005-2016 Hubert Figuiere
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

/**
 @brief Defines the class for reading TIFF-like file, including but not
 limited to TIFF, Exif, CR2, NEF, etc. It is designed to also address 
 issues like sone RAW file that do create veriation of TIFF just to confuse
 readers (like Olympus ORW).
*/


#ifndef OR_INTERNALS_IFDFILECONTAINER_H_
#define OR_INTERNALS_IFDFILECONTAINER_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <vector>

#include <libopenraw/consts.h>

#include "rawcontainer.hpp"
#include "ifddir.hpp"
#include "io/stream.hpp"

namespace OpenRaw {
namespace Internals {
		

class IfdFileContainer
  : public RawContainer
{
public:
  /** 
      constructor
      @param file the file handle
      @param offset the offset from the start of the file
  */
  IfdFileContainer(const IO::Stream::Ptr &file,
                   off_t offset);
  /** destructor */
  virtual ~IfdFileContainer();

  /** 
      due to the way Exif works, we have to set specific index
      to address these IFD 
  */
  enum {
    IFD_NONE = -1, /**< invalid IFD. Means an error */
    IFD_EXIF = -2, /**< exif IFD: see field 0x6789 in IFD 0 */
    IFD_GPS = -3,  /**< GPS IFD: see field 0x8825 in IFD 0 */
    IFD_INTEROP = -4 /**< interoperability IFD: see field 0xa005 in exif IFD*/ 
  };

  /** 
      Check the IFD magic header
		
      @param p the pointer to check
      @param len the length of the block to check. Likely to be at least 4.
      @return the endian if it is the magic header 
		
      subclasses needs to override it for like Olympus RAW
  */
  virtual EndianType isMagicHeader(const char *p, int len);
	
  /**
     Set the current directory
     @param dir the index of the directory to read, or one of the specific
     IFD index values that are < -1
     @return NULL if no error, or return the reference to the current directory
  */
  IfdDir::Ref setDirectory(int dir);
  /**
     Count the number of image file directories, not including
     EXIF, GPS and INTEROP.
     @return the number of image file directories
  */
  int countDirectories(void);
  /** Get the directories, loading them if necessary
   * @return the directories
   */
  std::vector<IfdDir::Ref> & directories();

  /**
     Get the number of the current directory
     @return the index of the current directory. By default we
     are in directory 0. -1 indicates an initialized IFD file
  */
  int currentDirectory();

  /**
   * get the extra data size chunk associated to the current image directory
   * @return the size of the data chunk in bytes
   */
  size_t getDirectoryDataSize();


  /**
     Return the last error
     @return the error code
  */
  int lastError() const
    {
      return m_error;
    }

  /**
     Return the Exif offset from the container begining. By 
     default it is 0, but some format like MRW needs a different one.
     This is an adjustement for the offset in the Exif IFD tag.
  */
  int exifOffsetCorrection() const
    {
      return m_exif_offset_correction;
    }

  /** Set the exif offset if needed. */
  void setExifOffsetCorrection(int corr)
    {
      m_exif_offset_correction = corr;
    }

  /** locate image data in the ifd (excepted RAW)
   * @param dir the IFD dir to locate the image data in
   * @param t the type of the image data
   * @return an error code
   */
  ::or_error locateImageData(const IfdDir::Ref& dir, uint32_t& x, uint32_t& y, 
                              ::or_data_type& t);
  
protected:
  /** hook to be called at the start of _locateDirs() */
  virtual bool locateDirsPreHook();
private:
  int m_error;
  int m_exif_offset_correction;

  IfdDir::Ref m_current_dir;
  std::vector<IfdDir::Ref> m_dirs;

  bool _locateDirs();
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
