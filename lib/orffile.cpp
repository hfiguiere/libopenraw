/*
 * libopenraw - orffile.cpp
 *
 * Copyright (C) 2006, 2008, 2010-2012 Hubert Figuiere
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

#include "trace.h"
#include "orffile.h"
#include "ifd.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "orfcontainer.h"
#include "olympusdecompressor.h"
#include "rawfile_private.h"
#include "io/file.h"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_OLYMPUS_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E1), 0, 0,
      { 11846,-4767,-945,-7027,15878,1089,-2699,4122,8311 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E10), 0, 0xffc,
      { 12745,-4500,-1416,-6062,14542,1580,-1934,2256,6603 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E3), 0, 0xf99,
      { 9487,-2875,-1115,-7533,15606,2010,-1618,2100,7389 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E5), 0, 0,
      { 11200,-3783,-1325,-4576,12593,2206,-695,1742,7504 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E300), 0, 0,
      { 7828,-1761,-348,-5788,14071,1830,-2853,4518,6557 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E330), 0, 0,
      { 8961,-2473,-1084,-7979,15990,2067,-2319,3035,8249 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E400), 0, 0,
      { 6169,-1483,-21,-7107,14761,2536,-2904,3580,8568 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E410), 0, 0xf6a,
      { 8856,-2582,-1026,-7761,15766,2082,-2009,2575,7469 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E500), 0, 0,
      { 8136,-1968,-299,-5481,13742,1871,-2556,4205,6630 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E510), 0, 0xf6a,
      { 8785,-2529,-1033,-7639,15624,2112,-1783,2300,7817 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E620), 0, 0xfaf,
      { 8453,-2198,-1092,-7609,15681,2008,-1725,2337,7824 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP350), 0, 0,
      { 12078,-4836,-1069,-6671,14306,2578,-786,939,7418 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP500), 0, 0xfff,
      { 9493,-3415,-666,-5211,12334,3260,-1548,2262,6482 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP510), 0, 0xffe,
      { 10593,-3607,-1010,-5881,13127,3084,-1200,1805,6721 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP550), 0, 0xffe,
      { 11597,-4006,-1049,-5432,12799,2957,-1029,1750,6516 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP1), 0, 0xffd,
      { 8343,-2050,-1021,-7715,15705,2103,-1831,2380,8235 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP2), 0, 0xffd,
      { 8343,-2050,-1021,-7715,15705,2103,-1831,2380,8235 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP3), 0, 0,
      { 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL1), 0, 0,
      { 11408,-4289,-1215,-4286,12385,2118,-387,1467,7787 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL2), 0, 0,
      { 15030,-5552,-1806,-3987,12387,1767,-592,1670,7023 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL3), 0, 0,
      { 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPM1), 0, 0,
      { 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ1), 0, 0,
      { 10901,-4095,-1074,-1141,9208,2293,-62,1417,5158 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5), 0, 0,
      { 8380, -2630, -639, -2887, 725, 2496, -627, 1427, 5438 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

};

const struct IfdFile::camera_ids_t OrfFile::s_def[] = {
    { "E-1             ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E1) },
    { "E-10        "    , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E10) },
    { "E-3             ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E3) },
    { "E-5             ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E5) },
    { "E-300           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E300) },
    { "E-330           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E330) },
    { "E-400           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E400) },
    { "E-410           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E410) },
    { "E-500           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E500) },
    { "E-510           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E510) },
    { "E-620           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E620) },
    { "SP350"           , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP350) },
    { "SP500UZ"         , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP500) },
    { "SP510UZ"         , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP510) },
    { "SP550UZ                ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP550) },
    { "E-P1            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP1) },
    { "E-P2            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP2) },
    { "E-P3            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP3) },
    { "E-PL1           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL1) },
    { "E-PL2           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL2) },
    { "E-PL3           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL3) },
    { "E-PM1           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPM1) },
    { "XZ-1            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ1) },
    { "XZ-2            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ2) },
    { "E-M5            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5) },
    { 0, 0 }
};

RawFile *OrfFile::factory(IO::Stream *s)
{
    return new OrfFile(s);
}

OrfFile::OrfFile(IO::Stream *s)
    : IfdFile(s, OR_RAWFILE_TYPE_ORF, false)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
    m_container = new OrfContainer(m_io, 0);
}

OrfFile::~OrfFile()
{
}

IfdDir::Ref  OrfFile::_locateCfaIfd()
{
    // in ORF the CFA IFD is the main IFD
    return mainIfd();
}


IfdDir::Ref  OrfFile::_locateMainIfd()
{
    return m_container->setDirectory(0);
}



::or_error OrfFile::_getRawData(RawData & data, uint32_t options)
{
    ::or_error err;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    err = _getRawDataFromDir(data, _cfaIfd);
    if(err == OR_ERROR_NONE) {
        // ORF files seems to be marked as uncompressed even if they are.
        uint32_t x = data.width();
        uint32_t y = data.height();
        uint32_t compression = 0;
        if(data.size() < x * y * 2) {
            compression = ORF_COMPRESSION;
            data.setCompression(ORF_COMPRESSION);
            data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);
        }
        else {
            compression = data.compression();
        }
        switch(compression) {
        case ORF_COMPRESSION:
            if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
                OlympusDecompressor decomp((const uint8_t*)data.data(), m_container, x, y);
                RawData *dData = decomp.decompress(NULL);
                if (dData != NULL) {
                    dData->setCfaPatternType(data.cfaPattern()->patternType());
                    data.swap(*dData);
                    data.setDataType(OR_DATA_TYPE_RAW);
                    data.setDimensions(x, y);
                    delete dData;
                }						
            }
            break;
        default:
            break;
        }
    }
    return err;
}

uint32_t OrfFile::_translateCompressionType(IFD::TiffCompress tiffCompression)
{
    if(tiffCompression == IFD::COMPRESS_CUSTOM) {
        return ORF_COMPRESSION;
    }
    return (uint32_t)tiffCompression;
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
