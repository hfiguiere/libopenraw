/*
 * libopenraw - orffile.cpp
 *
 * Copyright (C) 2006-2022 Hubert Figuière
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

#include <algorithm>
#include <cstdint>
#include <memory>

#include <libopenraw/debug.h>
#include <libopenraw/cameraids.h>
#include <libopenraw/consts.h>

#include "mosaicinfo.hpp"
#include "rawdata.hpp"
#include "rawfile.hpp"

#include "ifdfilecontainer.hpp"
#include "makernotedir.hpp"

#include "trace.hpp"
#include "orffile.hpp"
#include "ifd.hpp"
#include "ifddir.hpp"
#include "ifdentry.hpp"
#include "orfcontainer.hpp"
#include "olympusdecompressor.hpp"
#include "rawfile_private.hpp"
#include "io/streamclone.hpp"
#include "jfifcontainer.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_OLYMPUS_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E1),
      0,
      0,
      { 11846,-4767,-945,-7027,15878,1089,-2699,4122,8311 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E10),
      0,
      0xffc,
      { 12745,-4500,-1416,-6062,14542,1580,-1934,2256,6603 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E3),
      0,
      0xf99,
      { 9487,-2875,-1115,-7533,15606,2010,-1618,2100,7389 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E30),
      0,
      0,
      { 8144, -1861, -1111, -7763, 15894, 1929, -1865, 2542, 7607 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E5),
      0,
      0,
      { 11200,-3783,-1325,-4576,12593,2206,-695,1742,7504 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E300),
      0,
      0,
      { 7828,-1761,-348,-5788,14071,1830,-2853,4518,6557 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E330),
      0,
      0,
      { 8961,-2473,-1084,-7979,15990,2067,-2319,3035,8249 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E400),
      0,
      0,
      { 6169,-1483,-21,-7107,14761,2536,-2904,3580,8568 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E410),
      0,
      0xf6a,
      { 8856,-2582,-1026,-7761,15766,2082,-2009,2575,7469 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E420),
      0,
      0,
      { 8745, -2425, -1095, -7594, 15613, 2073, -1780, 2309, 7416 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E450),
      0,
      0,
      { 8745, -2425, -1095, -7594, 15613, 2073, -1780, 2309, 7416 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E500),
      0,
      0,
      { 8136,-1968,-299,-5481,13742,1871,-2556,4205,6630 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E510),
      0,
      0xf6a,
      { 8785,-2529,-1033,-7639,15624,2112,-1783,2300,7817 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E520),
      0,
      0,
      { 8343, -2322, -1020, -7596, 15635, 2048, -1748, 2269, 7287 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E600),
      0,
      0,
      { 8453, -2198, -1092, -7609, 15681, 2008, -1725, 2337, 7824 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E620),
      0,
      0xfaf,
      { 8453,-2198,-1092,-7609,15681,2008,-1725,2337,7824 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP350),
      0,
      0,
      { 12078,-4836,-1069,-6671,14306,2578,-786,939,7418 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP500UZ),
      0,
      0xfff,
      { 9493,-3415,-666,-5211,12334,3260,-1548,2262,6482 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP510UZ),
      0,
      0xffe,
      { 10593,-3607,-1010,-5881,13127,3084,-1200,1805,6721 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP550UZ),
      0,
      0xffe,
      { 11597,-4006,-1049,-5432,12799,2957,-1029,1750,6516 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP565UZ),
      0,
      0xffe,
      { 11856, -4470, -1159, -4814, 12368, 2756, -994, 1780, 5589 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP1),
      0,
      0xffd,
      { 8343,-2050,-1021,-7715,15705,2103,-1831,2380,8235 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP2),
      0,
      0xffd,
      { 8343,-2050,-1021,-7715,15705,2103,-1831,2380,8235 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP3),
      0,
      0,
      { 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP5),
      0,
      0,
      { 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL1),
      0,
      0,
      { 11408,-4289,-1215,-4286,12385,2118,-387,1467,7787 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL2),
      0,
      0,
      { 15030,-5552,-1806,-3987,12387,1767,-592,1670,7023 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL3),
      0,
      0,
      { 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL5),
      0,
      0xfcb,
      { 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL6),
      0,
      0xfcb,
      { 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL7),
      0,
      0xfcb,
      { 9197,-3190,-659,-2606,10830,2039,-458,1250,5458 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL8),
      0,
      0xfcb,
      { 9197, -3190, -659, -2606, 10830, 2039, -458, 1250, 5458 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL9),
      0,
      0xfcb,
      { 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL10),
      0,
      0xfcb,
      { 9197, -3190, -659, -2606, 10830, 2039, -458, 1250, 5458 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPM1),
      0,
      0,
      { 7575,-2159,-571,-3722,11341,2725,-1434,2819,6271 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPM2),
      0,
      0,
      { 8380,-2630,-639,-2887,10725,2496,-627,1427,5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ1),
      0,
      0,
      { 10901,-4095,-1074,-1141,9208,2293,-62,1417,5158 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ10),
      0,
      0,
      { 9777, -3483, -925, -2886, 11297, 1800, -602, 1663, 5134 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ2),
      0,
      0,
      { 9777,-3483,-925,-2886,11297,1800,-602,1663,5134 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5),
      0,
      0xfe1,
      { 8380, -2630, -639, -2887, 725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5II),
      0,
      0,
      { 9422, -3258, -711, -2655, 10898, 2015, -512, 1354, 5512 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5III),
      0,
      0,
      { 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1),
      0,
      0,
      { 7687, -1984, -606, -4327, 11928, 2721, -1381, 2339, 6452 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1II),
      0,
      0,
      { 9383, -3170, -763, -2457, 10702, 2020, -384, 1236, 5552 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1III),
      0,
      0,
      { 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10),
      0,
      0,
      { 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10II), // Identical to MarkI
      0,
      0,
      { 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10III),
      0,
      0,
      { 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10IIIS),
      0,
      0,
      { 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10IV),
      0,
      0,
      { 9476, -3182, -765, -2613, 10958, 1893, -449, 1315, 5268 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1X),
      0,
      0,
      { 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_OM1),
      0,
      0,
      { 9488, -3984, -714, -2887, 10945, 2229, -137, 960, 5786 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_STYLUS1),
      0,
      0,
      { 8360, -2420, -880, -3928, 12353, 1739, -1381, 2416, 5173 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_STYLUS1_1S),
      0,
      0,
      { 8360, -2420, -880, -3928, 12353, 1739, -1381, 2416, 5173 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_PEN_F),
      0,
      0,
      { 9476, -3182, -765, -2613, 10958, 1893, -449, 1315, 5268 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SH2),
      0,
      0,
      { 10156, -3425, -1077, -2611, 11177, 1624, -385, 1592, 5080 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_TG4),
      0,
      0,
      { 11426, -4159, -1126, -2066, 10678, 1593, -120, 1327, 4998 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_TG5),
      0,
      0,
      { 10899, -3833, -1082, -2112, 10736, 1575, -267, 1452, 5269 } },
    { OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_TG6),
      0,
      0,
      { 10899, -3833, -1082, -2112, 10736, 1575, -267, 1452, 5269 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

};

const struct IfdFile::camera_ids_t OrfFile::s_def[] = {
    { "E-1             ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E1) },
    { "E-10        "    , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E10) },
    { "E-3             ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E3) },
    { "E-30            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E30) },
    { "E-5             ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E5) },
    { "E-300           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E300) },
    { "E-330           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E330) },
    { "E-400           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E400) },
    { "E-410           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E410) },
    { "E-420           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E420) },
    { "E-450           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E450) },
    { "E-500           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E500) },
    { "E-510           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E510) },
    { "E-520           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E520) },
    { "E-600           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E600) },
    { "E-620           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_E620) },
    { "SP350"           , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP350) },
    { "SP500UZ"         , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP500UZ) },
    { "SP510UZ"         , OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP510UZ) },
    { "SP550UZ                ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP550UZ) },
    { "SP565UZ                ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SP565UZ) },
    { "E-P1            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP1) },
    { "E-P2            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP2) },
    { "E-P3            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP3) },
    { "E-P5            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EP5) },
    { "E-PL1           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL1) },
    { "E-PL2           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL2) },
    { "E-PL3           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL3) },
    { "E-PL5           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL5) },
    { "E-PL6           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL6) },
    { "E-PL7           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL7) },
    { "E-PL8           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL8) },
    { "E-PL9           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL9) },
    { "E-PL10          ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPL10) },
    { "E-PM1           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPM1) },
    { "E-PM2           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EPM2) },
    { "XZ-1            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ1) },
    { "XZ-10           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ10) },
    { "XZ-2            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_XZ2) },
    { "E-M5            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5) },
    { "E-M5MarkII      ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5II) },
    { "E-M5MarkIII     ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM5III) },
    { "E-M1            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1) },
    { "E-M1MarkII      ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1II) },
    { "E-M1MarkIII     ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1III) },
    { "E-M1X           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM1X) },
    { "E-M10           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10) },
    { "E-M10MarkII     ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10II) },
    { "E-M10 Mark III  ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10III) },
    { "E-M10MarkIIIS   ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10IIIS) },
    { "E-M10MarkIV     ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_EM10IV) },
    { "OM-1            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_OM1) },
    { "STYLUS1         ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_STYLUS1) },
    { "STYLUS1,1s      ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_STYLUS1_1S) },
    { "PEN-F           ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_PEN_F) },
    { "SH-2            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_SH2) },
    { "TG-4            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_TG4) },
    { "TG-5            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_TG5) },
    { "TG-6            ", OR_MAKE_OLYMPUS_TYPEID(OR_TYPEID_OLYMPUS_TG6) },
    { 0, 0 }
};

RawFile *OrfFile::factory(const IO::Stream::Ptr &s)
{
    return new OrfFile(s);
}

OrfFile::OrfFile(const IO::Stream::Ptr &s)
    : IfdFile(s, OR_RAWFILE_TYPE_ORF, false)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
    m_container = new OrfContainer(m_io, 0);
}

OrfFile::~OrfFile()
{
}

::or_error OrfFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
    auto err = OR_ERROR_NOT_FOUND;

    err = IfdFile::_enumThumbnailSizes(list);
    LOGDBG1("got %lu thumbs\n", (LSIZE)list.size());

    auto exif = exifIfd();
    if (!exif) {
        LOGDBG1("can't get exif\n");
        return err;
    }

    auto makerNote = std::dynamic_pointer_cast<MakerNoteDir>(exif->getMakerNoteIfd(type()));
    if (!makerNote) {
        LOGDBG1("can't get makernote\n");
        return err;
    }

    auto e = makerNote->getEntry(ORF_TAG_THUMBNAIL_IMAGE);
    _addThumbnailFromEntry(e, makerNote->getMnoteOffset(), list);

    auto ifd = makerNote->getIfdInEntry(ORF_TAG_CAMERA_SETTINGS);
    if (ifd) {
        LOGDBG1("CameraSettings %lu\n", (LSIZE)ifd->entries().size());
        uint32_t is_valid = ifd->getValue<uint32_t>(ORF_TAG_CS_PREVIEW_IMAGE_VALID).value_or(0);
        if (is_valid) {
            uint32_t start =
                ifd->getValue<uint32_t>(ORF_TAG_CS_PREVIEW_IMAGE_START).value_or(0);
            if (start) {
                start += makerNote->getMnoteOffset();
            }
            uint32_t len = ifd->getValue<uint32_t>(ORF_TAG_CS_PREVIEW_IMAGE_LENGTH).value_or(0);
            LOGDBG1("is_valid %u, start %u len %u\n", is_valid, start, len);

            // if either value is zero we consider it is wrong.
            if (start != 0 && len != 0) {
                err = _addThumbnailFromStream(start, len, list);
            } else {
                err = OR_ERROR_NOT_FOUND;
            }
        }
    }

    return err;
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
            compression = IFD::COMPRESS_OLYMPUS;
            data.setCompression(IFD::COMPRESS_OLYMPUS);
            data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);
        }
        else {
            compression = data.compression();
        }
        switch(compression) {
        case IFD::COMPRESS_OLYMPUS:
            if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
                OlympusDecompressor decomp((const uint8_t*)data.data(),
                                           data.size(), m_container, x, y);
                RawDataPtr dData = decomp.decompress();
                if (dData) {
                    dData->setCfaPatternType(data.mosaicInfo()->patternType());
                    data.swap(*dData);
                    data.setDataType(OR_DATA_TYPE_RAW);
                    data.setDimensions(x, y);
                }
            }
            break;
        default:
            break;
        }
    }
    return err;
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
