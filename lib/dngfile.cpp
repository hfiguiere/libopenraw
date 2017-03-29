/*
 * libopenraw - dngfile.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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

#include <stddef.h>

#include <string>
#include <memory>

#include <libopenraw/cameraids.h>
#include <libopenraw/debug.h>

#include "rawdata.hpp"
#include "trace.hpp"
#include "io/memstream.hpp"
#include "jfifcontainer.hpp"
#include "ljpegdecompressor.hpp"
#include "ifd.hpp"
#include "ifddir.hpp"
#include "ifdentry.hpp"
#include "dngfile.hpp"

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
    { "PENTAX Q7          ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_Q7_DNG) },
    { "PENTAX QS-1        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_QS1_DNG) },
    { "PENTAX K-x         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_KX_DNG) },
    { "PENTAX K-r         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_KR_DNG) },
    { "PENTAX K-01        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K01_DNG) },
    { "PENTAX K-1         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K1_DNG) },
    { "PENTAX K-30        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K30_DNG) },
    { "PENTAX K-5 II s    ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K5_IIS_DNG) },
    { "PENTAX K-50        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K50_DNG) },
    { "PENTAX K-500       ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K500_DNG) },
    { "PENTAX K-3         ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K3_DNG) },
    { "PENTAX K-3 II      ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K3_II_DNG) },
    { "PENTAX K-70        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_K70_DNG) },
    { "PENTAX K-S1        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_KS1_DNG) },
    { "PENTAX KP          ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,
                                                 OR_TYPEID_PENTAX_KP_DNG) },
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
    { "LEICA M10",      OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_M10) },
    { "LEICA X1               ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_X1) },
    { "LEICA X2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                      OR_TYPEID_LEICA_X2) },
    { "Leica S2", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_S2) },
    { "LEICA X VARIO (Typ 107)", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                                     OR_TYPEID_LEICA_X_VARIO) },
    { "LEICA X (Typ 113)", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_X_TYP113) },
    { "LEICA SL (Typ 601)", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_SL_TYP601) },
    { "LEICA T (Typ 701)", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_T_TYP701) },
    { "LEICA Q (Typ 116)", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,
                                               OR_TYPEID_LEICA_Q_TYP116) },
    { "GR DIGITAL 2   ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
                                             OR_TYPEID_RICOH_GR2) },
    { "GR                                                             ",
                         OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
                                             OR_TYPEID_RICOH_GR) },
    { "GXR            ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,
                                             OR_TYPEID_RICOH_GXR) },
    { "GXR A16                                                        ",
      OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH, OR_TYPEID_RICOH_GXR_A16) },
    { "SAMSUNG GX10       ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SAMSUNG,
                                                 OR_TYPEID_SAMSUNG_GX10) },
    { "Pro 815    ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SAMSUNG,
                                         OR_TYPEID_SAMSUNG_PRO815) },
    { "M1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_XIAOYI,
                                         OR_TYPEID_XIAOYI_M1) },
    { "iPhone 6s Plus", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_APPLE,
                                         OR_TYPEID_APPLE_IPHONE_6SPLUS) },
    { "iPhone 7 Plus", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_APPLE,
                                         OR_TYPEID_APPLE_IPHONE_7PLUS) },
    { 0, OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_ADOBE,
                             OR_TYPEID_ADOBE_DNG_GENERIC) }
};

RawFile *DngFile::factory(const IO::Stream::Ptr &s)
{
    return new DngFile(s);
}


DngFile::DngFile(const IO::Stream::Ptr &s)
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

    LOGDBG1("_getRawData()\n");

    if (!_cfaIfd) {
        LOGDBG1("cfaIfd is NULL: not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    ret = _getRawDataFromDir(data, _cfaIfd);

    if(ret != OR_ERROR_NONE) {
        LOGERR("couldn't find raw data\n");
        return ret;
    }

    auto result = _cfaIfd->getValue<uint16_t>(IFD::EXIF_TAG_COMPRESSION);
    if (result.ok() && (result.unwrap() == IFD::COMPRESS_LJPEG)) {
        // if the option is not set, decompress
        if ((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
            IO::Stream::Ptr s(
                std::make_shared<IO::MemStream>(data.data(),
                                                data.size()));
            s->open(); // TODO check success
            std::unique_ptr<JfifContainer> jfif(new JfifContainer(s, 0));
            LJpegDecompressor decomp(s.get(), jfif.get());
            RawDataPtr dData = decomp.decompress();
            if (dData) {
                dData->setCfaPattern(data.cfaPattern());
                data.swap(*dData);
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

    return ret;
}

void DngFile::_identifyId()
{
    TiffEpFile::_identifyId();
    // XXX what if the DNG file match the non DNG?
    // XXX maybe we should hint of the type in the camera ID table
    if (OR_GET_FILE_TYPEID_CAMERA(_typeId()) == 0) {
        const IfdDir::Ref & _mainIfd = mainIfd();

        auto uniqueCameraModel =
            _mainIfd->getValue<std::string>(IFD::DNG_TAG_UNIQUE_CAMERA_MODEL);
        if (uniqueCameraModel.ok()) {
            // set a generic DNG type if we found a
            // unique camera model
            _setTypeId(
                OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_ADOBE,
                                    OR_TYPEID_ADOBE_DNG_GENERIC));
        }
    }
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
