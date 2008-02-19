/*
 * libopenraw - ifdfilecontainer.h
 *
 * Copyright (C) 2005-2006 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 @brief Defines the class for reading TIFF-like file, including but not
 limited to TIFF, Exif, CR2, NEF, etc. It is designed to also address 
 issues like sone RAW file that do create veriation of TIFF just to confuse
 readers (like Olympus ORW).
*/


#ifndef _IFDFILECONTAINER_H_
#define _IFDFILECONTAINER_H_

#include <vector>

#include <libopenraw/consts.h>
#include <libopenraw/types.h>

#include "rawcontainer.h"
#include "ifddir.h"
#include "ifdentry.h"

namespace OpenRaw {
	namespace Internals {
		

		class IFDFileContainer
			: public RawContainer
		{
		public:
			/** 
					constructor
					@param file the file handle
					@param offset the offset from the start of the file
			*/
			IFDFileContainer(IO::Stream *file, off_t offset);
			/** destructor */
			virtual ~IFDFileContainer();

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
			IFDDir::Ref setDirectory(int dir);
			/**
				 Count the number of image file directories, not including
				 EXIF, GPS and INTEROP.
				 @return the number of image file directories
			*/
			int countDirectories(void);
			/** Get the directories, loading them if necessary
			 * @return the directories
			 */
			std::vector<IFDDir::Ref> & directories();

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

			int exifOffsetCorrection() const
				{
					return m_exif_offset_correction;
				}
			void setExifOffsetCorrection(int corr)
				{
					m_exif_offset_correction = corr;
				}
		protected:
			/** hook to be called at the start of _locateDirs() */
			virtual bool locateDirsPreHook();
		private:
			int m_error;
			int m_exif_offset_correction;

			IFDDir::Ref m_current_dir;
			std::vector<IFDDir::Ref> m_dirs;

			bool _locateDirs();
		};

	}
}

#endif
