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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
			IFDFileContainer(IOFile *file, off_t offset);
			/** destructor */
			virtual ~IFDFileContainer();

			/** define the endian of the container */
			typedef enum {
				ENDIAN_NULL = 0, /** no endian found: means invalid file */
				ENDIAN_BIG,      /** big endian found */
				ENDIAN_LITTLE    /** little endian found */
			} EndianType;
			/** 
					due to the way Exif works, we have to set specific index
					to address these IFD 
			*/
			enum {
				IFD_NONE = -1, /** invalid IFD. Means an error */
				IFD_EXIF = -2, /** exif IFD: see field 0x6789 in IFD 0 */
				IFD_GPS = -3,  /** GPS IFD: see field 0x8825 in IFD 0 */
				IFD_INTEROP = -4 /** interoperability IFD: see field 0xa005 in exif IFD*/ 
			};

			/** identify a tag */
			typedef short int tag_t;

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
			 * Fetch the data chunnk from the current image directory
			 * @param buf the buffer to load into
			 * @param offset the offset
			 * @param buf_size the size of the data to fetch
			 * @return the size retrieved, <= buf_size likely equal
			 */
			size_t fetchData(void *buf, const off_t offset, const size_t buf_size);


			/**
				 Get the IFD field tagged by tag
				 @param tag the tag to get
				 @param value a value returning the field content
				 @return the error code
			*/
			bool getField(const tag_t tag, IFDEntry &value);

			/**
				 Return the last error
				 @return the error code
			*/
			int lastError() const
				{
					return m_error;
				}

			/** Read an int16 following the m_endian set */
			bool readInt16(IOFile *f, Int16 & v);
			/** Read an int32 following the m_endian set */
			bool readInt32(IOFile *f, Int32 & v);
		private:
			int m_error;
			EndianType m_endian;

			IFDDir::Ref m_current_dir;
			std::vector<IFDDir::Ref> m_dirs;

			bool _locateDirs();
		};

	}
}

#endif
