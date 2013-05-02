/*
 * libopenraw - dngfile.cpp
 *
 * Copyright (C) 2006-2008, 2011-2013 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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


#include <libopenraw/cameraids.h>

#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include <boost/scoped_ptr.hpp>

#include "trace.h"
#include "io/file.h"
#include "io/memstream.h"
#include "ifdfilecontainer.h"
#include "jfifcontainer.h"
#include "ljpegdecompressor.h"
#include "ifd.h"
#include "dngfile.h"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

const IfdFile::camera_ids_t DngFile::s_def[] = {
    { "PENTAX K10D        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K10D_DNG) },
    { "PENTAX Q           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_Q_DNG) },
    { "PENTAX K200D       ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K200D_DNG) },
    { "PENTAX Q10         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_Q10_DNG) },
    { "PENTAX K-x         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_KX_DNG) },
    { "PENTAX K-r         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_KR_DNG) },
    { "PENTAX K-01        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K01_DNG) },
    { "PENTAX K-30        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K30_DNG) },
    { "PENTAX K-5 II s    ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K5_IIS_DNG) },
    { "PENTAX MX-1            ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_MX1_DNG) },
    { "R9 - Digital Back DMR",   OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_DMR) },
    { "M8 Digital Camera",       OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_M8) },
    { "M9 Digital Camera",       OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_M9) },
    { "M Monochrom",       OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_M_MONOCHROM) },
    { "LEICA M (Typ 240)",       OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_M_TYP240) },
    { "LEICA X1               ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_X1) },
    { "LEICA X2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                      OR_TYPEID_LEICA_X2) },
    { "Leica S2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_S2) },
    { "GR DIGITAL 2   ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
                                             OR_TYPEID_RICOH_GR2) },
    { "GXR            ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
                                             OR_TYPEID_RICOH_GXR) },
    { "GXR A16                                                        ", 
      OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH, OR_TYPEID_RICOH_GXR_A16) },
    { "SAMSUNG GX10       ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SAMSUNG,
                                                 OR_TYPEID_SAMSUNG_GX10) },
    { "Pro 815    ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SAMSUNG, 
                                         OR_TYPEID_SAMSUNG_PRO815) },
    { 0, OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_ADOBE, 
                             OR_TYPEID_ADOBE_DNG_GENERIC) }
};

RawFile *DngFile::factory(IO::Stream *s)
{
    return new DngFile(s);
}


DngFile::DngFile(IO::Stream *s)
    : TiffEpFile(s, OR_RAWFILE_TYPE_DNG)
{
    _setIdMap(s_def);
}

DngFile::~DngFile()
{
}

::or_error DngFile::_getRawData(RawData & data, uint32_t options)
{
    ::or_error ret = OR_ERROR_NONE;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    
    Trace(DEBUG1) << "_getRawData()\n";
    
    if (_cfaIfd) {
        ret = _getRawDataFromDir(data, _cfaIfd);
	
        if(ret == OR_ERROR_NONE) {
            uint16_t compression = 0;
            if (_cfaIfd->getValue(IFD::EXIF_TAG_COMPRESSION, compression) &&
                compression == IFD::COMPRESS_LJPEG) {
                // if the option is not set, decompress
                if ((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
                    boost::scoped_ptr<IO::Stream> s(new IO::MemStream(data.data(),
                                                                      data.size()));
                    s->open(); // TODO check success
                    boost::scoped_ptr<JfifContainer> jfif(new JfifContainer(s.get(), 0));
                    LJpegDecompressor decomp(s.get(), jfif.get());
                    RawData *dData = decomp.decompress();
                    if (dData != NULL) {
                        dData->setCfaPattern(data.cfaPattern());
                        data.swap(*dData);
                        delete dData;
                    }
                }
            }
            else {
                data.setDataType(OR_DATA_TYPE_RAW);
            }
            uint32_t crop_x, crop_y, crop_w, crop_h;
            IfdEntry::Ref e = _cfaIfd->getEntry(IFD::DNG_TAG_DEFAULT_CROP_ORIGIN);
            if(e) {
                crop_x = e->getIntegerArrayItem(0);
                crop_y = e->getIntegerArrayItem(1);
            }
            else {
                crop_x = crop_y = 0;
            }
            e = _cfaIfd->getEntry(IFD::DNG_TAG_DEFAULT_CROP_SIZE);
            if(e) {
                crop_w = e->getIntegerArrayItem(0);
                crop_h = e->getIntegerArrayItem(1);
            }
            else {
                crop_w = data.width();
                crop_h = data.height();
            }
            data.setRoi(crop_x, crop_y, crop_w, crop_h);
        }
        else {
            Trace(ERROR) << "couldn't find raw data\n";
        }
    }
    else {
        ret = OR_ERROR_NOT_FOUND;
    }
    return ret;
}

}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
