/*
 * libopenraw - peffile.cpp
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


#include <libopenraw/cameraids.h>

#include "rawdata.hpp"
#include "ifd.hpp"
#include "ifdfilecontainer.hpp"
#include "ifddir.hpp"
#include "peffile.hpp"
#include "rawfile_private.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_PENTAX_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,camid)
#define OR_MAKE_RICOH_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_RICOH,camid)

static const ModelIdMap modelid_map = {
    { 0x12994, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_D_PEF) },
    { 0x12aa2, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DS_PEF) },
    { 0x12b1a, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL_PEF) },
    // *ist DS2
    { 0x12b7e, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL2_PEF) },
    { 0x12b9c, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF) },
    { 0x12b9d, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K110D_PEF) },
    { 0x12ba2, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_SUPER_PEF) },
    { 0x12c1e, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K10D_PEF) },
    { 0x12cd2, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K20D_PEF) },
    { 0x12cfa, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K200D_PEF) },
    { 0x12d72, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K2000_PEF) },
    { 0x12d73, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KM_PEF) },
    { 0x12db8, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K7_PEF) },
    { 0x12dfe, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KX_PEF) },
    { 0x12e08, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_645D_PEF) },
    { 0x12e6c, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KR_PEF) },
    { 0x12e76, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_PEF) },
    // Q
    // K-01
    // K-30
    // Q10
    { 0x12f70, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_II_PEF) },
    { 0x12f71, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_IIS_PEF) },
    // Q7
    // K-50
    { 0x12fc0, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_PEF) },
    // K-500
    { 0x13010, OR_MAKE_RICOH_TYPEID(OR_TYPEID_PENTAX_645Z_PEF) },
    { 0x1301a, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KS1_PEF) },
    { 0x13024, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KS2_PEF) },
    // Q-S1
    { 0x13092, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K1_PEF) },
    { 0x1309c, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_II_PEF) },
    // GR III
    { 0x13222, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K70_PEF) },
    { 0x1322c, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KP_PEF) },
    { 0x13240, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K1_MKII_PEF) },
    { 0x13254, OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_MKIII_PEF) },
};

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_D_PEF),
      0,
      0,
      { 9651, -2059, -1189, -8881, 16512, 2487, -1460, 1345, 10687 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL_PEF),
      0,
      0,
      { 10829, -2838, -1115, -8339, 15817, 2696, -837, 680, 11939 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL2_PEF),
      0,
      0,
      { 10504, -2439, -1189, -8603, 16208, 2531, -1022, 863, 12242 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DS_PEF),
      0,
      0,
      { 10371, -2333, -1206, -8688, 16231, 2602, -1230, 1116, 11282 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K10D_PEF),
      0,
      0,
      { 9566, -2863, -803, -7170, 15172, 2112, -818, 803, 9705 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K1_PEF),
      0,
      0,
      { 8566, -2746, -1201, -3612, 12204, 1550, -893, 1680, 6264 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K1_MKII_PEF),
      0,
      0,
      { 8596, -2981, -639, -4202, 12046, 2431, -685, 1424, 6122 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF),
      0,
      0,
      { 11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_SUPER_PEF),
      0,
      0,
      { 11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K110D_PEF),
      0,
      0,
      { 11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K20D_PEF),
      0,
      0,
      { 9427, -2714, -868, -7493, 16092, 1373, -2199, 3264, 7180 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K200D_PEF),
      0,
      0,
      { 9186, -2678, -907, -8693, 16517, 2260, -1129, 1094, 8524 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K2000_PEF),
      0,
      0,
      { 9730, -2989, -970, -8527, 16258, 2381, -1060, 970, 8362 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KR_PEF),
      0,
      0,
      { 9895, -3077, -850, -5304, 13035, 2521, -883, 1768, 6936 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_PEF),
      0,
      0,
      { 8542, -2581, -1144, -3995, 12301, 1881, -863, 1514, 5755 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_II_PEF),
      0,
      0,
      { 9251, -3817, -1069, -4627, 12667, 2175, -798, 1660, 5633 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_MKIII_PEF),
      0,
      0,
      { 8571, -2590, -1148, -3995, 12301, 1881, -1052, 1844, 7013 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_PEF),
      0,
      0,
      { 8713, -2833, -743, -4342, 11900, 2772, -722, 1543, 6247 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_II_PEF),
      0,
      0,
      { 8435, -2549, -1130, -3995, 12301, 1881, -989, 1734, 6591 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_IIS_PEF),
      0,
      0,
      { 8170, -2725, -639, -4440, 12017, 2744, -771, 1465, 6599 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K7_PEF),
      0,
      0,
      { 9142, -2947, -678, -8648, 16967, 1663, -2224, 2898, 8615 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K70_PEF),
      0,
      0,
      { 8766, -3149, -747, -3976, 11943, 2292, -517, 1259, 5552 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KM_PEF),
      0,
      0,
      { 9730, -2989, -970, -8527, 16258, 2381, -1060, 970, 8362 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KX_PEF),
      0,
      0,
      { 8843, -2837, -625, -5025, 12644, 2668, -411, 1234, 7410 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KS1_PEF),
      0,
      0,
      { 7989, -2511, -1137, -3882, 12350, 1689, -862, 1524, 6444 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KS2_PEF),
      0,
      0,
      { 8662, -3280, -798, -3928, 11771, 2444, -586, 1232, 6054 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KP_PEF),
      0,
      0,
      { 8617, -3228, -1034, -4674, 12821, 2044, -803, 1577, 5728 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_645D_PEF),
      0,
      0x3e00,
      { 10646, -3593, -1158, -3329, 11699, 1831, -667, 2874, 6287 } },
    { OR_MAKE_RICOH_TYPEID(OR_TYPEID_PENTAX_645Z_PEF),
      0,
      0x3fff,
      { 9519, -3591, -664, -4074, 11725, 2671, -624, 1501, 6653 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const struct IfdFile::camera_ids_t PEFFile::s_def[] = {
    { "PENTAX *ist D      ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_D_PEF) },
    { "PENTAX *ist DL     ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL_PEF) },
    { "PENTAX *ist DL2    ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL2_PEF) },
    { "PENTAX *ist DS     ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DS_PEF) },
    { "PENTAX K10D        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K10D_PEF) },
    { "PENTAX K100D       ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF) },
    { "PENTAX K100D Super ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_SUPER_PEF) },
    { "PENTAX K110D       ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K110D_PEF) },
    { "PENTAX K20D        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K20D_PEF) },
    { "PENTAX K200D       ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K200D_PEF) },
    { "PENTAX K2000       ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K2000_PEF) },
    { "PENTAX K-1         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K1_PEF) },
    { "PENTAX K-1 Mark II ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K1_MKII_PEF) },
    { "PENTAX K-r         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KR_PEF) },
    { "PENTAX K-3         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_PEF) },
    { "PENTAX K-3 II      ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_II_PEF) },
    { "PENTAX K-3 Mark III             ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K3_MKIII_PEF) },
    { "PENTAX K-5         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_PEF) },
    { "PENTAX K-5 II      ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_II_PEF) },
    { "PENTAX K-5 II s    ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_IIS_PEF) },
    { "PENTAX K-7         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K7_PEF) },
    { "PENTAX K-70        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K70_PEF) },
    { "PENTAX K-S1        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KS1_PEF) },
    { "PENTAX K-S2        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KS2_PEF) },
    { "PENTAX K-m         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KM_PEF) },
    { "PENTAX K-x         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KX_PEF) },
    { "PENTAX KP          ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KP_PEF) },
    { "PENTAX 645D        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_645D_PEF) },
    { "PENTAX 645Z        ", OR_MAKE_RICOH_TYPEID(OR_TYPEID_PENTAX_645Z_PEF) },
    { 0, 0 }
};


RawFile *PEFFile::factory(const IO::Stream::Ptr &s)
{
    return new PEFFile(s);
}

PEFFile::PEFFile(const IO::Stream::Ptr &s)
    : IfdFile(s, OR_RAWFILE_TYPE_PEF)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

PEFFile::~PEFFile()
{
}

::or_error PEFFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
    auto err = this->IfdFile::_enumThumbnailSizes(list);
    auto makerNote = makerNoteIfd();
    if (makerNote) {
        auto e = makerNote->getEntry(MNOTE_PENTAX_PREVIEW_IMAGE_SIZE);
        if (e) {
            auto w = makerNote->getEntryValue<uint16_t>(*e, 0);
            auto h = makerNote->getEntryValue<uint16_t>(*e, 1);
            auto dim = std::max(w, h);
            list.push_back(dim);

            auto offset = makerNote->getIntegerValue(MNOTE_PENTAX_PREVIEW_IMAGE_START).value_or(0);
            if (offset > 0) {
                offset += makerNote->getMnoteOffset();
            }
            auto length = makerNote->getIntegerValue(MNOTE_PENTAX_PREVIEW_IMAGE_LENGTH).value_or(0);
            if (offset != 0 && length != 0) {
                _addThumbnail(dim, ThumbDesc(w, h, OR_DATA_TYPE_JPEG, offset, length));
                err = OR_ERROR_NONE;
            }
        }

    }

    return err;
}

bool PEFFile::vendorCameraIdLocation(Internals::IfdDir::Ref& ifd, uint16_t& index,
                                     const ModelIdMap*& model_map)
{
    auto mn = makerNoteIfd();
    if (mn) {
        // There is a camera model ID in the MakerNote tag 0x0010.
        ifd = mn;
        index = IFD::MNOTE_PENTAX_MODEL_ID;
        model_map = &modelid_map;
        return true;
    }
    return false;
}

::or_error PEFFile::_getRawData(RawData & data, uint32_t options)
{
    ::or_error err;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    err = _getRawDataFromDir(data, _cfaIfd);
    if (err == OR_ERROR_NONE) {
        auto mnote = makerNoteIfd();
        auto offset = mnote->getEntry(MNOTE_PENTAX_IMAGEAREAOFFSET);
        if (offset) {
            auto x = mnote->getEntryValue<uint16_t>(*offset, 0);
            auto y = mnote->getEntryValue<uint16_t>(*offset, 1);
            auto image_size = mnote->getEntry(MNOTE_PENTAX_RAWIMAGESIZE);
            if (image_size) {
                auto w = mnote->getEntryValue<uint16_t>(*image_size, 0);
                auto h = mnote->getEntryValue<uint16_t>(*image_size, 1);
                data.setActiveArea(x, y, w, h);
            }
        }

        auto white_level = mnote->getIntegerValue(MNOTE_PENTAX_WHITELEVEL);
        if (white_level) {
            data.setWhiteLevel(white_level.value());
        }

        uint16_t compression = data.compression();
        switch(compression) {
        case IFD::COMPRESS_CUSTOM:
            if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
                // TODO decompress
            }
            break;
        case IFD::COMPRESS_PENTAX_PACK:
            break;
        case IFD::COMPRESS_NONE:
        {
            // Pentax a big endian. This should be done in _getRawDataFromDir() according
            // to the container endian.
            auto p = data.data();
            uint16_t* pixels = (uint16_t*)p;
            for (size_t i = 0; i < data.size() / 2; i++) {
                *pixels = be16toh(*pixels);
                pixels++;
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

