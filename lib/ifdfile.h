/*
 * libopenraw - ifdfile.h
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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



#ifndef _IFD_FILE_H_
#define _IFD_FILE_H_

#include <vector>
#include <libopenraw/types.h>
#include <libopenraw/consts.h>
#include <libopenraw++/rawfile.h>

#include "ifddir.h"
#include "makernotedir.h"

namespace OpenRaw {
namespace IO {
	class Stream;
	class File;
}

namespace Internals {
class IfdFileContainer;

/** describe the location of a thumbnail in an IFD file */
struct IfdThumbDesc
{
	IfdThumbDesc(uint32_t _x, uint32_t _y, ::or_data_type _type,
							 const IfdDir::Ref & _ifddir)
		: x(_x), y(_y), type(_type), ifddir(_ifddir)
		{
		}
	IfdThumbDesc()
		: x(0), y(0), type(OR_DATA_TYPE_NONE), ifddir()
		{
		}
	uint32_t x;    /**< x size. Can be 0 */
	uint32_t y;    /**< y size. Can be 0 */
	::or_data_type type; /**< the data type format */
	IfdDir::Ref ifddir; /**< the IFD directory */
};


/** @brief generic IFD based raw file. */
class IfdFile
	: public OpenRaw::RawFile
{

protected:
	IfdFile(IO::Stream * s, Type _type, 
			bool instantiateContainer = true);
	virtual ~IfdFile();

	/** list the thumbnails in the IFD
	 * @retval list the list of thumbnails
	 * @return the error code. OR_ERROR_NOT_FOUND if no
	 * thumbnail are found. 
	 */
	virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list);

	/** locate the thumnail in the IFD 
	 * @param dir the IfdDir where to locate the thumbnail
	 * @return the error code. OR_ERROR_NOT_FOUND if the
	 * thumbnail are not found.
	 */
	virtual ::or_error _locateThumbnail(const IfdDir::Ref & dir,
										std::vector<uint32_t> &list);
	/** load the compressed rawdata from a standard location in an IFD
	 * @param data the data storage
	 * @param dir the IFD
	 * @return the error code.
	 */
	::or_error _getRawDataFromDir(RawData & data, const IfdDir::Ref & dir);
	
	/** Get the JPEG thumbnail offset from dir.
	 * @param dir the IFD to get the thumbnail from
	 * @param len the length of the JPEG stream. 0 is not valid.
	 * @return the offset. 0 is not valid.
	 */
	virtual uint32_t _getJpegThumbnailOffset(const IfdDir::Ref & dir, uint32_t & len);

	typedef std::map<uint32_t, IfdThumbDesc> ThumbLocations;
	ThumbLocations    m_thumbLocations;
	IO::Stream       *m_io; /**< the IO handle */
	IfdFileContainer *m_container; /**< the real container */

	virtual IfdDir::Ref  _locateCfaIfd() = 0;
	virtual IfdDir::Ref  _locateMainIfd() = 0;
	virtual IfdDir::Ref  _locateExifIfd();
	virtual MakerNoteDir::Ref  _locateMakerNoteIfd();

	virtual void _identifyId();

	virtual MetaValue *_getMetaValue(int32_t meta_index);
	
	/** Translate the compression type from the tiff type (16MSB) 
	 * to the RAW specific type if needed (16MSB)
	 * @param tiffCompression the 16 bits value from TIFF
	 * @return the actually value. Anything >= 2^16 is specific the RAW type
	 */
	virtual uint32_t _translateCompressionType(IFD::TiffCompress tiffCompression);

	/** access the corresponding IFD. Will locate them if needed */
	const IfdDir::Ref & cfaIfd();
	const IfdDir::Ref & mainIfd();
	const IfdDir::Ref & exifIfd();
	const MakerNoteDir::Ref & makerNoteIfd();			
private:
	IfdDir::Ref       m_cfaIfd;  /**< the IFD for the CFA */
	IfdDir::Ref       m_mainIfd; /**< the IFD for the main image 
								  * does not necessarily reference 
								  * the CFA
								  */
	IfdDir::Ref       m_exifIfd; /**< the Exif IFD */
	MakerNoteDir::Ref m_makerNoteIfd; /**< the MakerNote IFD */
	
	IfdFile(const IfdFile&);
	IfdFile & operator=(const IfdFile &);

	virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail);
};

}
}


#endif
