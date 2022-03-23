/*
 * libopenraw - cameraids.h
 *
 * Copyright (C) 2012-2022 Hubert Figuière
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
 * @file The libopenraw list of camera ID.
 * @author Hubert Figuière <hub@figuiere.net>
 */

#ifndef LIBOPENRAW_CAMERAIDS_H_
#define LIBOPENRAW_CAMERAIDS_H_

#include <stdint.h>

/** @defgroup camera_id Camera IDs
 * @ingroup public_api
 *
 * @brief The vendor and camera IDs.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief The vendor ID: the high order 16-bits of the or_rawfile_typeid
 */
enum _OR_TYPE_ID_VENDOR {
    OR_TYPEID_VENDOR_NONE = 0,
    OR_TYPEID_VENDOR_CANON = 1,
    OR_TYPEID_VENDOR_NIKON = 2,
    OR_TYPEID_VENDOR_LEICA = 3,
    OR_TYPEID_VENDOR_PENTAX = 4,
    OR_TYPEID_VENDOR_EPSON = 5,
    OR_TYPEID_VENDOR_MINOLTA = 6,
    OR_TYPEID_VENDOR_OLYMPUS = 7,
    OR_TYPEID_VENDOR_SONY = 8,
    OR_TYPEID_VENDOR_SAMSUNG = 9,
    OR_TYPEID_VENDOR_RICOH = 10,
    OR_TYPEID_VENDOR_PANASONIC = 11,
    OR_TYPEID_VENDOR_MAMIYA = 12,
    OR_TYPEID_VENDOR_ADOBE = 13, /**< Generic DNG files. */
    OR_TYPEID_VENDOR_FUJIFILM = 14,
    OR_TYPEID_VENDOR_BLACKMAGIC = 15,
    OR_TYPEID_VENDOR_XIAOYI = 16,
    OR_TYPEID_VENDOR_APPLE = 17,
    OR_TYPEID_VENDOR_SIGMA = 18,
    OR_TYPEID_VENDOR_GOPRO = 19,
    OR_TYPEID_VENDOR_HASSELBLAD = 20,
    OR_TYPEID_VENDOR_ZEISS = 21,

    _OR_TYPEID_VENDOR_LAST
};

enum { OR_TYPEID_UNKNOWN = 0 };

/** @brief Adobe type ID */
enum _OR_TYPEID_VENDOR_ADOBE {
    OR_TYPEID_ADOBE_UNKNOWN = 0,
    OR_TYPEID_ADOBE_DNG_GENERIC = 1, /**< Generic DNG file. */
    _OR_TYPEID_ADOBE_LAST
};

/** @brief Canon type IDs */
enum _OR_TYPEID_VENDOR_CANON {
    OR_TYPEID_CANON_UNKNOWN = 0,
    OR_TYPEID_CANON_20D = 1,
    OR_TYPEID_CANON_30D = 2,
    OR_TYPEID_CANON_40D = 3,
    OR_TYPEID_CANON_350D = 4,
    OR_TYPEID_CANON_REBEL_XT = OR_TYPEID_CANON_350D,
    OR_TYPEID_CANON_KISS_DIGITAL_N = OR_TYPEID_CANON_350D,
    OR_TYPEID_CANON_400D = 5,
    OR_TYPEID_CANON_REBEL_XTI = OR_TYPEID_CANON_400D,
    OR_TYPEID_CANON_KISS_DIGITAL_X = OR_TYPEID_CANON_400D,
    OR_TYPEID_CANON_450D = 6,
    OR_TYPEID_CANON_REBEL_XSI = OR_TYPEID_CANON_450D,
    OR_TYPEID_CANON_KISS_X2 = OR_TYPEID_CANON_450D,
    OR_TYPEID_CANON_5D = 7,
    OR_TYPEID_CANON_1D = 8,
    OR_TYPEID_CANON_1DMKII = 9,
    OR_TYPEID_CANON_1DMKIII = 10,
    OR_TYPEID_CANON_1DS = 11,
    OR_TYPEID_CANON_1DSMKII = 12,
    OR_TYPEID_CANON_1DSMKIII = 13,
    OR_TYPEID_CANON_300D = 14,
    OR_TYPEID_CANON_DIGITAL_REBEL = OR_TYPEID_CANON_300D,
    OR_TYPEID_CANON_D30 = 15,
    OR_TYPEID_CANON_D60 = 16,
    OR_TYPEID_CANON_10D = 17,
    OR_TYPEID_CANON_PRO1 = 18,
    OR_TYPEID_CANON_G1 = 19,
    OR_TYPEID_CANON_G2 = 20,
    OR_TYPEID_CANON_G3 = 21,
    OR_TYPEID_CANON_G5 = 22,
    OR_TYPEID_CANON_G6 = 23,
    OR_TYPEID_CANON_G7 = 24,
    OR_TYPEID_CANON_G9 = 25,
    OR_TYPEID_CANON_A610 = 26,
    OR_TYPEID_CANON_20DA = 27,
    OR_TYPEID_CANON_7D = 28,
    OR_TYPEID_CANON_G11 = 29,
    OR_TYPEID_CANON_1DMKIV = 30,
    OR_TYPEID_CANON_500D = 31,
    OR_TYPEID_CANON_REBEL_T1I = OR_TYPEID_CANON_500D,
    OR_TYPEID_CANON_KISS_X3 = OR_TYPEID_CANON_500D,
    OR_TYPEID_CANON_5DMKII = 32,
    OR_TYPEID_CANON_550D = 33,
    OR_TYPEID_CANON_REBEL_T2I = OR_TYPEID_CANON_550D,
    OR_TYPEID_CANON_KISS_X4 = OR_TYPEID_CANON_550D,
    OR_TYPEID_CANON_1000D = 34,
    OR_TYPEID_CANON_REBEL_XS = OR_TYPEID_CANON_1000D,
    OR_TYPEID_CANON_KISS_F = OR_TYPEID_CANON_1000D,
    OR_TYPEID_CANON_G10 = 35,
    OR_TYPEID_CANON_50D = 36,
    OR_TYPEID_CANON_60D = 36,
    OR_TYPEID_CANON_S90 = 37,
    OR_TYPEID_CANON_G12 = 38,
    OR_TYPEID_CANON_S95 = 39,
    OR_TYPEID_CANON_600D = 40,
    OR_TYPEID_CANON_REBEL_T3I = OR_TYPEID_CANON_600D,
    OR_TYPEID_CANON_KISS_X5 = OR_TYPEID_CANON_600D,
    OR_TYPEID_CANON_1100D = 41,
    OR_TYPEID_CANON_REBEL_T3 = OR_TYPEID_CANON_1100D,
    OR_TYPEID_CANON_KISS_X50 = OR_TYPEID_CANON_1100D,
    OR_TYPEID_CANON_G1X = 42,
    OR_TYPEID_CANON_S100 = 43,
    OR_TYPEID_CANON_5DMKIII = 44,
    OR_TYPEID_CANON_1DX = 45,
    OR_TYPEID_CANON_60Da = 46,
    OR_TYPEID_CANON_650D = 47,
    OR_TYPEID_CANON_REBEL_T4I = OR_TYPEID_CANON_650D,
    OR_TYPEID_CANON_KISS_X6I = OR_TYPEID_CANON_650D,
    OR_TYPEID_CANON_G15 = 48,
    OR_TYPEID_CANON_6D = 49,
    OR_TYPEID_CANON_EOS_M = 50,
    OR_TYPEID_CANON_SX50_HS = 51,
    OR_TYPEID_CANON_S110 = 52,
    OR_TYPEID_CANON_100D = 53,
    OR_TYPEID_CANON_REBEL_SL1 = OR_TYPEID_CANON_100D,
    OR_TYPEID_CANON_KISS_X7 = OR_TYPEID_CANON_100D,
    OR_TYPEID_CANON_700D = 54,
    OR_TYPEID_CANON_REBEL_T5I = OR_TYPEID_CANON_700D,
    OR_TYPEID_CANON_KISS_X7I = OR_TYPEID_CANON_700D,
    OR_TYPEID_CANON_G16 = 55,
    OR_TYPEID_CANON_70D = 56,
    OR_TYPEID_CANON_7DMKII = 57,
    OR_TYPEID_CANON_G7X = 58,
    OR_TYPEID_CANON_G1XMKII = 59,
    OR_TYPEID_CANON_750D = 60,
    OR_TYPEID_CANON_REBEL_T6I = OR_TYPEID_CANON_750D,
    OR_TYPEID_CANON_KISS_X8I = OR_TYPEID_CANON_750D,
    OR_TYPEID_CANON_760D = 61,
    OR_TYPEID_CANON_REBEL_T6S = OR_TYPEID_CANON_760D,
    OR_TYPEID_CANON_8000D = OR_TYPEID_CANON_760D,
    OR_TYPEID_CANON_5DS_R = 62,
    OR_TYPEID_CANON_G3X = 63,
    OR_TYPEID_CANON_G9XMKII = 64,
    OR_TYPEID_CANON_5DMKIV = 65,
    OR_TYPEID_CANON_EOS_M5 = 66,
    OR_TYPEID_CANON_G5X = 67,
    OR_TYPEID_CANON_G7XMKII = 68,
    OR_TYPEID_CANON_1300D = 69,
    OR_TYPEID_CANON_REBEL_T6 = OR_TYPEID_CANON_1300D,
    OR_TYPEID_CANON_KISS_X80 = OR_TYPEID_CANON_1300D,
    OR_TYPEID_CANON_EOS_M3 = 70,
    OR_TYPEID_CANON_1DXMKII = 71,
    OR_TYPEID_CANON_80D = 72,
    OR_TYPEID_CANON_1200D = 73,
    OR_TYPEID_CANON_REBEL_T5 = OR_TYPEID_CANON_1200D,
    OR_TYPEID_CANON_KISS_X70 = OR_TYPEID_CANON_1200D,
    OR_TYPEID_CANON_G9X = 74,
    OR_TYPEID_CANON_EOS_M10 = 75,
    OR_TYPEID_CANON_800D = 76,
    OR_TYPEID_CANON_REBEL_T7I = OR_TYPEID_CANON_800D,
    OR_TYPEID_CANON_KISS_X9I = OR_TYPEID_CANON_800D,
    OR_TYPEID_CANON_77D = 77,
    OR_TYPEID_CANON_9000D = OR_TYPEID_CANON_77D,
    OR_TYPEID_CANON_EOS_M6 = 78,
    OR_TYPEID_CANON_EOS_M100 = 79,
    OR_TYPEID_CANON_6DMKII = 80,
    OR_TYPEID_CANON_200D = 81,
    OR_TYPEID_CANON_REBEL_SL2 = OR_TYPEID_CANON_200D,
    OR_TYPEID_CANON_KISS_X9 = OR_TYPEID_CANON_200D,
    OR_TYPEID_CANON_G1XMKIII = 82,
    OR_TYPEID_CANON_5DS = 83,
    OR_TYPEID_CANON_EOS_M50 = 84,
    OR_TYPEID_CANON_KISS_M = OR_TYPEID_CANON_EOS_M50,
    OR_TYPEID_CANON_SX1_IS = 85,
    OR_TYPEID_CANON_S120 = 86,
    OR_TYPEID_CANON_SX60_HS = 87,
    OR_TYPEID_CANON_2000D = 88,
    OR_TYPEID_CANON_REBEL_T7 = OR_TYPEID_CANON_2000D,
    OR_TYPEID_CANON_KISS_X90 = OR_TYPEID_CANON_2000D,
    OR_TYPEID_CANON_EOS_R = 89,
    OR_TYPEID_CANON_3000D = 90,
    OR_TYPEID_CANON_T100 = OR_TYPEID_CANON_3000D,
    OR_TYPEID_CANON_4000D = OR_TYPEID_CANON_3000D,
    OR_TYPEID_CANON_1DMKIIN = 91,
    OR_TYPEID_CANON_SX70_HS = 92,
    OR_TYPEID_CANON_EOS_RP = 93,
    OR_TYPEID_CANON_250D = 94,
    OR_TYPEID_CANON_REBEL_SL3 = OR_TYPEID_CANON_250D,
    OR_TYPEID_CANON_KISS_X10 = OR_TYPEID_CANON_250D,
    OR_TYPEID_CANON_G7XMKIII = 95,
    OR_TYPEID_CANON_G5XMKII = 96,
    OR_TYPEID_CANON_EOS_M6MKII = 97,
    OR_TYPEID_CANON_90D = 98,
    OR_TYPEID_CANON_EOS_M200 = 99,
    OR_TYPEID_CANON_1DXMKIII = 100,
    OR_TYPEID_CANON_EOS_R5 = 101,
    OR_TYPEID_CANON_EOS_R6 = 102,
    OR_TYPEID_CANON_S30 = 103,
    OR_TYPEID_CANON_S40 = 104,
    OR_TYPEID_CANON_S45 = 105,
    OR_TYPEID_CANON_S50 = 106,
    OR_TYPEID_CANON_S60 = 107,
    OR_TYPEID_CANON_S70 = 108,
    OR_TYPEID_CANON_S100V = 109,
    OR_TYPEID_CANON_PRO70 = 110,
    OR_TYPEID_CANON_PRO90 = 111,
    OR_TYPEID_CANON_850D = 112,
    OR_TYPEID_CANON_REBEL_T8I = OR_TYPEID_CANON_850D,
    OR_TYPEID_CANON_KISS_X10I = OR_TYPEID_CANON_850D,
    _OR_TYPEID_CANON_LAST
};

/** @brief Nikon type IDs */
enum _OR_TYPEID_VENDOR_NIKON {
    OR_TYPEID_NIKON_UNKNOWN = 0,
    OR_TYPEID_NIKON_E5700,
    OR_TYPEID_NIKON_D1,
    OR_TYPEID_NIKON_D1X,
    OR_TYPEID_NIKON_D100,
    OR_TYPEID_NIKON_D2H,
    OR_TYPEID_NIKON_D2X,
    OR_TYPEID_NIKON_D200,
    OR_TYPEID_NIKON_D3,
    OR_TYPEID_NIKON_D300,
    OR_TYPEID_NIKON_D40,
    OR_TYPEID_NIKON_D40X,
    OR_TYPEID_NIKON_D50,
    OR_TYPEID_NIKON_D70,
    OR_TYPEID_NIKON_D70S,
    OR_TYPEID_NIKON_D80,
    OR_TYPEID_NIKON_D3000,
    OR_TYPEID_NIKON_COOLPIX_P6000,
    OR_TYPEID_NIKON_COOLPIX_P7000,
    OR_TYPEID_NIKON_D7000,
    OR_TYPEID_NIKON_D3100,
    OR_TYPEID_NIKON_1_J1,
    OR_TYPEID_NIKON_1_V1,
    OR_TYPEID_NIKON_COOLPIX_P7100,
    OR_TYPEID_NIKON_D5000,
    OR_TYPEID_NIKON_D5100,
    OR_TYPEID_NIKON_D4,
    OR_TYPEID_NIKON_D3S,
    OR_TYPEID_NIKON_D3X,
    OR_TYPEID_NIKON_D300S,
    OR_TYPEID_NIKON_D3200,
    OR_TYPEID_NIKON_D700,
    OR_TYPEID_NIKON_D800,
    OR_TYPEID_NIKON_D800E,
    OR_TYPEID_NIKON_D90,
    OR_TYPEID_NIKON_D600,
    OR_TYPEID_NIKON_COOLPIX_P7700,
    OR_TYPEID_NIKON_1_V2,
    OR_TYPEID_NIKON_D5200,
    OR_TYPEID_NIKON_D7100,
    OR_TYPEID_NIKON_COOLPIX_A,
    OR_TYPEID_NIKON_1_J2,
    OR_TYPEID_NIKON_1_J3,
    OR_TYPEID_NIKON_1_S1,
    OR_TYPEID_NIKON_D60,
    OR_TYPEID_NIKON_DF,
    OR_TYPEID_NIKON_E5400,
    OR_TYPEID_NIKON_E8400,
    OR_TYPEID_NIKON_D4S,
    OR_TYPEID_NIKON_D610,
    OR_TYPEID_NIKON_D750,
    OR_TYPEID_NIKON_1_J5,
    OR_TYPEID_NIKON_1_V3,
    OR_TYPEID_NIKON_D7200,
    OR_TYPEID_NIKON_D5300,
    OR_TYPEID_NIKON_D5500,
    OR_TYPEID_NIKON_D3300,
    OR_TYPEID_NIKON_D810,
    OR_TYPEID_NIKON_D5600,
    OR_TYPEID_NIKON_D3400,
    OR_TYPEID_NIKON_D5,
    OR_TYPEID_NIKON_D500,
    OR_TYPEID_NIKON_1_AW1,
    OR_TYPEID_NIKON_1_S2,
    OR_TYPEID_NIKON_1_J4,
    OR_TYPEID_NIKON_COOLPIX_B700,
    OR_TYPEID_NIKON_COOLPIX_P330,
    OR_TYPEID_NIKON_COOLPIX_P340,
    OR_TYPEID_NIKON_Z6,
    OR_TYPEID_NIKON_Z7,
    OR_TYPEID_NIKON_COOLPIX_P1000,
    OR_TYPEID_NIKON_E8800,
    OR_TYPEID_NIKON_D3500,
    OR_TYPEID_NIKON_D2HS,
    OR_TYPEID_NIKON_D2XS,
    OR_TYPEID_NIKON_COOLPIX_A1000,
    OR_TYPEID_NIKON_D780,
    OR_TYPEID_NIKON_Z50,
    OR_TYPEID_NIKON_COOLPIX_P950,
    OR_TYPEID_NIKON_D6,
    OR_TYPEID_NIKON_Z5,
    OR_TYPEID_NIKON_Z6_2,
    OR_TYPEID_NIKON_Z7_2,
    OR_TYPEID_NIKON_ZFC,
    OR_TYPEID_NIKON_Z9,
    _OR_TYPEID_NIKON_LAST
};

/** @brief Leica type IDs */
enum _OR_TYPEID_VENDOR_LEICA {
    OR_TYPEID_LEICA_UNKNOWN = 0,
    /* DNG */
    OR_TYPEID_LEICA_DMR = 1,
    OR_TYPEID_LEICA_M8 = 2,
    OR_TYPEID_LEICA_X1 = 3,
    /* Panasonic RAW */
    OR_TYPEID_LEICA_DIGILUX2 = 4,
    OR_TYPEID_LEICA_DLUX_3 = 5,
    OR_TYPEID_LEICA_VLUX_1 = 6,
    /* DNG */
    OR_TYPEID_LEICA_M9 = 7,
    OR_TYPEID_LEICA_S2 = 8,
    OR_TYPEID_LEICA_M_MONOCHROM = 9,
    OR_TYPEID_LEICA_X2 = 10,
    OR_TYPEID_LEICA_M_TYP240 = 11,
    OR_TYPEID_LEICA_X_VARIO = 12,
    OR_TYPEID_LEICA_T_TYP701 = 13,
    OR_TYPEID_LEICA_Q_TYP116 = 14,
    OR_TYPEID_LEICA_X_TYP113 = 15,
    OR_TYPEID_LEICA_M10 = 16,
    OR_TYPEID_LEICA_SL_TYP601 = 17,
    /* RWL (Panasonic RAW) */
    OR_TYPEID_LEICA_DLUX_TYP109 = 18,
    OR_TYPEID_LEICA_VLUX_4 = 19,
    /* DNG */
    OR_TYPEID_LEICA_CL = 20,
    /* RWL (Panasonic RAW) */
    OR_TYPEID_LEICA_VLUX_TYP114 = 21,
    OR_TYPEID_LEICA_CLUX = 22,
    /* DNG */
    OR_TYPEID_LEICA_M10P = 23,
    OR_TYPEID_LEICA_M10D = 24,
    OR_TYPEID_LEICA_TL2 = 25,
    /* RWL */
    OR_TYPEID_LEICA_DLUX_5 = 26,
    OR_TYPEID_LEICA_DLUX_7 = 27,
    OR_TYPEID_LEICA_C_TYP112 = 28,
    OR_TYPEID_LEICA_VLUX_5 = 29,
    /* DNG */
    OR_TYPEID_LEICA_Q2 = 30,
    OR_TYPEID_LEICA_SL2 = 31,
    OR_TYPEID_LEICA_M10_MONOCHROM = 32,
    OR_TYPEID_LEICA_M10R = 33,
    OR_TYPEID_LEICA_M_MONOCHROM_TYP246 = 34,
    /* RWL */
    OR_TYPEID_LEICA_DLUX_4 = 35,
    /* DNG */
    OR_TYPEID_LEICA_SL2S = 36,
    OR_TYPEID_LEICA_Q2_MONOCHROM = 37,
    _OR_TYPEID_LEICA_LAST
};

/** @brief Pentax type IDs */
enum _OR_TYPEID_VENDOR_PENTAX {
    OR_TYPEID_PENTAX_UNKNOWN = 0,
    OR_TYPEID_PENTAX_K10D_PEF,
    OR_TYPEID_PENTAX_K10D_DNG,
    OR_TYPEID_PENTAX_IST_D_PEF,
    OR_TYPEID_PENTAX_IST_DL_PEF,
    OR_TYPEID_PENTAX_K100D_PEF,
    OR_TYPEID_PENTAX_K100D_SUPER_PEF,
    OR_TYPEID_PENTAX_K20D_PEF,
    OR_TYPEID_PENTAX_KR_PEF,
    OR_TYPEID_PENTAX_KX_PEF,
    OR_TYPEID_PENTAX_K5_PEF,
    OR_TYPEID_PENTAX_K7_PEF,
    OR_TYPEID_PENTAX_645D_PEF,
    OR_TYPEID_PENTAX_645D_DNG,
    OR_TYPEID_PENTAX_K2000_DNG,
    OR_TYPEID_PENTAX_Q_DNG,
    OR_TYPEID_PENTAX_K200D_DNG,
    OR_TYPEID_PENTAX_KM_PEF,
    OR_TYPEID_PENTAX_KX_DNG,
    OR_TYPEID_PENTAX_KR_DNG,
    OR_TYPEID_PENTAX_K01_DNG,
    OR_TYPEID_PENTAX_K30_DNG,
    OR_TYPEID_PENTAX_K5_IIS_DNG,
    OR_TYPEID_PENTAX_MX1_DNG,
    OR_TYPEID_PENTAX_Q10_DNG,
    OR_TYPEID_PENTAX_Q7_DNG,
    OR_TYPEID_PENTAX_K3_DNG,
    OR_TYPEID_PENTAX_K50_DNG,
    OR_TYPEID_PENTAX_K500_DNG,
    OR_TYPEID_PENTAX_K200D_PEF,
    OR_TYPEID_PENTAX_IST_DS_PEF,
    OR_TYPEID_PENTAX_K5_IIS_PEF,
    OR_TYPEID_PENTAX_K3_II_DNG,
    OR_TYPEID_PENTAX_K1_PEF,
    OR_TYPEID_PENTAX_K1_DNG,
    OR_TYPEID_PENTAX_K70_PEF,
    OR_TYPEID_PENTAX_K70_DNG,
    OR_TYPEID_PENTAX_KS1_PEF,
    OR_TYPEID_PENTAX_KS1_DNG,
    OR_TYPEID_PENTAX_KS2_PEF,
    OR_TYPEID_PENTAX_KS2_DNG,
    OR_TYPEID_PENTAX_QS1_DNG,
    OR_TYPEID_PENTAX_QS1_PEF,
    OR_TYPEID_PENTAX_KP_PEF,
    OR_TYPEID_PENTAX_KP_DNG,
    OR_TYPEID_PENTAX_K1_MKII_PEF,
    OR_TYPEID_PENTAX_K1_MKII_DNG,
    OR_TYPEID_PENTAX_K7_DNG,
    OR_TYPEID_PENTAX_IST_DL2_PEF,
    OR_TYPEID_PENTAX_K5_II_PEF,
    OR_TYPEID_PENTAX_K5_II_DNG,
    OR_TYPEID_PENTAX_K3_PEF,
    OR_TYPEID_PENTAX_K3_II_PEF,
    OR_TYPEID_PENTAX_K110D_PEF,
    _OR_TYPEID_PENTAX_LAST
};

/** @brief Epson type IDs */
enum _OR_TYPEID_VENDOR_EPSON {
    OR_TYPEID_EPSON_UNKNOWN = 0,
    OR_TYPEID_EPSON_RD1 = 1,
    OR_TYPEID_EPSON_RD1S = 2,
    _OR_TYPEID_EPSON_LAST
};

/** @brief Minolta type IDs */
enum _OR_TYPEID_VENDOR_MINOLTA {
    OR_TYPEID_MINOLTA_UNKNOWN = 0,
    OR_TYPEID_MINOLTA_A1,
    OR_TYPEID_MINOLTA_A2,
    OR_TYPEID_MINOLTA_DIMAGE5,
    OR_TYPEID_MINOLTA_DIMAGE7,
    OR_TYPEID_MINOLTA_DIMAGE7I,
    OR_TYPEID_MINOLTA_DIMAGE7HI,
    OR_TYPEID_MINOLTA_MAXXUM_5D,
    OR_TYPEID_MINOLTA_MAXXUM_7D,
    OR_TYPEID_MINOLTA_A200,
    _OR_TYPEID_MINOLTA_LAST
};

/** @brief Olympus type IDs */
enum _OR_TYPEID_VENDOR_OLYMPUS {
    OR_TYPEID_OLYMPUS_UNKNOWN = 0,
    OR_TYPEID_OLYMPUS_E1,
    OR_TYPEID_OLYMPUS_E10,
    OR_TYPEID_OLYMPUS_E3,
    OR_TYPEID_OLYMPUS_E300,
    OR_TYPEID_OLYMPUS_E330,
    OR_TYPEID_OLYMPUS_E400,
    OR_TYPEID_OLYMPUS_E410,
    OR_TYPEID_OLYMPUS_E500,
    OR_TYPEID_OLYMPUS_E510,
    OR_TYPEID_OLYMPUS_SP350,
    OR_TYPEID_OLYMPUS_SP510UZ,
    OR_TYPEID_OLYMPUS_SP550UZ,
    OR_TYPEID_OLYMPUS_SP500UZ,
    OR_TYPEID_OLYMPUS_EP1,
    OR_TYPEID_OLYMPUS_E620,
    OR_TYPEID_OLYMPUS_EPL1,
    OR_TYPEID_OLYMPUS_EP2,
    OR_TYPEID_OLYMPUS_XZ1,
    OR_TYPEID_OLYMPUS_E5,
    OR_TYPEID_OLYMPUS_EPL2,
    OR_TYPEID_OLYMPUS_EP3,
    OR_TYPEID_OLYMPUS_EPL3,
    OR_TYPEID_OLYMPUS_EPM1,
    OR_TYPEID_OLYMPUS_EM5,
    OR_TYPEID_OLYMPUS_XZ2,
    OR_TYPEID_OLYMPUS_EPM2,
    OR_TYPEID_OLYMPUS_EPL5,
    OR_TYPEID_OLYMPUS_EM1,
    OR_TYPEID_OLYMPUS_STYLUS1,
    OR_TYPEID_OLYMPUS_EPL6,
    OR_TYPEID_OLYMPUS_EPL7,
    OR_TYPEID_OLYMPUS_EM5II,
    OR_TYPEID_OLYMPUS_EM1II,
    OR_TYPEID_OLYMPUS_PEN_F,
    OR_TYPEID_OLYMPUS_EM10,
    OR_TYPEID_OLYMPUS_EM10II,
    OR_TYPEID_OLYMPUS_EPL8,
    OR_TYPEID_OLYMPUS_SH2,
    OR_TYPEID_OLYMPUS_XZ10,
    OR_TYPEID_OLYMPUS_TG4,
    OR_TYPEID_OLYMPUS_EPL9,
    OR_TYPEID_OLYMPUS_STYLUS1_1S,
    OR_TYPEID_OLYMPUS_EM10III,
    OR_TYPEID_OLYMPUS_TG5,
    OR_TYPEID_OLYMPUS_EM1X,
    OR_TYPEID_OLYMPUS_TG6,
    OR_TYPEID_OLYMPUS_EM5III,
    OR_TYPEID_OLYMPUS_SP565UZ,
    OR_TYPEID_OLYMPUS_EPL10,
    OR_TYPEID_OLYMPUS_EM1III,
    OR_TYPEID_OLYMPUS_EM10IV,

    _OR_TYPEID_OLYMPUS_LAST
};

/** @brief Samsung type IDs */
enum _OR_TYPEID_VENDOR_SAMSUNG {
    OR_TYPEID_SAMSUNG_UNKNOWN = 0,
    OR_TYPEID_SAMSUNG_GX10,
    OR_TYPEID_SAMSUNG_PRO815,
    _OR_TYPEID_SAMSUNG_LAST
};

/** @brief Ricoh type IDs
 *
 * Following the merger with Pentax newer cameras may be Pentax.
 */
enum _OR_TYPEID_VENDOR_RICOH {
    OR_TYPEID_RICOH_UNKNOWN = 0,
    OR_TYPEID_RICOH_GR2 = 1,
    OR_TYPEID_RICOH_GXR = 2,
    OR_TYPEID_RICOH_GXR_A16 = 3,
    OR_TYPEID_RICOH_GR = 4, /* 2013 Ricoh GR-D */
    OR_TYPEID_RICOH_GX200 = 5,
    OR_TYPEID_PENTAX_645Z_PEF = 6,
    OR_TYPEID_PENTAX_645Z_DNG = 7,
    OR_TYPEID_RICOH_GRII = 8,
    OR_TYPEID_RICOH_GRIII = 9,
    _OR_TYPEID_RICOH_LAST
};

/** @brief Sony type IDs */
enum _OR_TYPEID_VENDOR_SONY {
    OR_TYPEID_SONY_UNKNOWN = 0,
    OR_TYPEID_SONY_A100 = 1,
    OR_TYPEID_SONY_A200 = 2,
    OR_TYPEID_SONY_A700 = 3,
    OR_TYPEID_SONY_A550 = 4,
    OR_TYPEID_SONY_A380 = 5,
    OR_TYPEID_SONY_A390 = OR_TYPEID_SONY_A380,
    OR_TYPEID_SONY_SLTA55 = 7,
    OR_TYPEID_SONY_SLTA77 = 8,
    OR_TYPEID_SONY_NEX3 = 9,
    OR_TYPEID_SONY_NEX3N = 10,
    OR_TYPEID_SONY_NEX5 = 11,
    OR_TYPEID_SONY_NEX5N = 12,
    OR_TYPEID_SONY_NEX5R = 13,
    OR_TYPEID_SONY_NEX5T = 14,
    OR_TYPEID_SONY_NEX6 = 15,
    OR_TYPEID_SONY_NEX7 = 16,
    OR_TYPEID_SONY_NEXC3 = 17,
    OR_TYPEID_SONY_NEXF3 = 18,
    OR_TYPEID_SONY_SLTA65 = 19,
    OR_TYPEID_SONY_A330 = 21,
    OR_TYPEID_SONY_A350 = 21,
    OR_TYPEID_SONY_A450 = 22,
    OR_TYPEID_SONY_A580 = 23,
    OR_TYPEID_SONY_A850 = 24,
    OR_TYPEID_SONY_A900 = 25,
    OR_TYPEID_SONY_SLTA35 = 26,
    OR_TYPEID_SONY_SLTA33 = 27,
    OR_TYPEID_SONY_A560 = 28,
    OR_TYPEID_SONY_SLTA99 = 29,
    OR_TYPEID_SONY_RX100 = 30,
    OR_TYPEID_SONY_RX100M2 = 31,
    OR_TYPEID_SONY_RX100M3 = 32,
    OR_TYPEID_SONY_RX100M4 = 33,
    OR_TYPEID_SONY_RX100M5 = 34,
    OR_TYPEID_SONY_RX100M6 = 35,
    OR_TYPEID_SONY_RX1 = 36,
    OR_TYPEID_SONY_RX1R = 37,
    OR_TYPEID_SONY_RX10 = 38,
    OR_TYPEID_SONY_RX10M2 = 39,
    OR_TYPEID_SONY_RX10M3 = 40,
    OR_TYPEID_SONY_RX1RM2 = 41,
    OR_TYPEID_SONY_RX10M4 = 42,
    OR_TYPEID_SONY_RX0 = 43,
    OR_TYPEID_SONY_SLTA57 = 44,
    OR_TYPEID_SONY_ILCE7 = 45,
    OR_TYPEID_SONY_ILCE7M2 = 46,
    OR_TYPEID_SONY_ILCE7M3 = 47,
    OR_TYPEID_SONY_ILCE7R = 48,
    OR_TYPEID_SONY_ILCE7RM2 = 49,
    OR_TYPEID_SONY_ILCE7RM3 = 50,
    OR_TYPEID_SONY_ILCE7S = 51,
    OR_TYPEID_SONY_ILCE7SM2 = 52,
    OR_TYPEID_SONY_ILCE9 = 53,
    OR_TYPEID_SONY_ILCE3000 = 54,
    OR_TYPEID_SONY_ILCE3500 = OR_TYPEID_SONY_ILCE3000,
    OR_TYPEID_SONY_SLTA58 = 55,
    OR_TYPEID_SONY_ILCE6000 = 56,
    OR_TYPEID_SONY_ILCA99M2 = 57,
    OR_TYPEID_SONY_ILCE6300 = 58,
    OR_TYPEID_SONY_ILCE6500 = 59,
    OR_TYPEID_SONY_ILCE5100 = 60,
    OR_TYPEID_SONY_A230 = 61,
    OR_TYPEID_SONY_A500 = 62,
    OR_TYPEID_SONY_SLTA37 = 63,
    OR_TYPEID_SONY_ILCA77M2 = 64,
    OR_TYPEID_SONY_ILCA68 = 65,
    OR_TYPEID_SONY_ILCE5000 = 66,
    OR_TYPEID_SONY_A290 = 67,
    OR_TYPEID_SONY_RX100M5A = 68,
    OR_TYPEID_SONY_HX99 = 69,
    OR_TYPEID_SONY_HX95 = OR_TYPEID_SONY_HX99,
    OR_TYPEID_SONY_ILCE6400 = 70,
    OR_TYPEID_SONY_RX0M2 = 71,
    OR_TYPEID_SONY_ILCE7RM4 = 72,
    OR_TYPEID_SONY_RX100M7 = 73,
    OR_TYPEID_SONY_ILCE6100 = 74,
    OR_TYPEID_SONY_ILCE6600 = 75,
    OR_TYPEID_SONY_ILCE9M2 = 76,
    OR_TYPEID_SONY_ZV1 = 77,
    // SR2 file
    OR_TYPEID_SONY_R1 = 78,
    // ARW
    OR_TYPEID_SONY_ILCE7SM3 = 79,
    OR_TYPEID_SONY_ILCE7C = 80,
    _OR_TYPEID_SONY_LAST
};

/** @brief Panasonic type IDs */
enum _OR_TYPEID_VENDOR_PANASONIC {
    OR_TYPEID_PANASONIC_UNKNOWN = 0,
    OR_TYPEID_PANASONIC_GF1 = 1,
    OR_TYPEID_PANASONIC_GF2 = 2,
    OR_TYPEID_PANASONIC_FZ30 = 3,
    OR_TYPEID_PANASONIC_G10 = 4,
    OR_TYPEID_PANASONIC_GH1 = 5,
    OR_TYPEID_PANASONIC_GH2 = 6,
    OR_TYPEID_PANASONIC_LX2 = 7,
    OR_TYPEID_PANASONIC_LX3 = 8,
    OR_TYPEID_PANASONIC_LX5 = 9,
    OR_TYPEID_PANASONIC_FZ8 = 10,
    OR_TYPEID_PANASONIC_FZ18 = 11,
    OR_TYPEID_PANASONIC_FZ50 = 12,
    OR_TYPEID_PANASONIC_L1 = 13,
    OR_TYPEID_PANASONIC_G1 = 14,
    OR_TYPEID_PANASONIC_G2 = 15,
    OR_TYPEID_PANASONIC_L10 = 16,
    OR_TYPEID_PANASONIC_FZ28 = 17,
    OR_TYPEID_PANASONIC_GF3 = 18,
    OR_TYPEID_PANASONIC_FZ100 = 19,
    OR_TYPEID_PANASONIC_GX1 = 20,
    OR_TYPEID_PANASONIC_G3 = 21,
    OR_TYPEID_PANASONIC_G5 = 22,
    OR_TYPEID_PANASONIC_GF5 = 23,
    OR_TYPEID_PANASONIC_LX7 = 24,
    OR_TYPEID_PANASONIC_GH3 = 25,
    OR_TYPEID_PANASONIC_FZ200 = 26,
    OR_TYPEID_PANASONIC_GF6 = 27,
    OR_TYPEID_PANASONIC_GX7 = 28,
    OR_TYPEID_PANASONIC_GM1 = 29,
    OR_TYPEID_PANASONIC_GH4 = 30,
    OR_TYPEID_PANASONIC_LX100 = 31,
    OR_TYPEID_PANASONIC_GM5 = 32,
    OR_TYPEID_PANASONIC_G80 = 33,
    OR_TYPEID_PANASONIC_G85 = OR_TYPEID_PANASONIC_G80,
    OR_TYPEID_PANASONIC_LX10 = 34,
    OR_TYPEID_PANASONIC_LX15 = OR_TYPEID_PANASONIC_LX10,
    OR_TYPEID_PANASONIC_FZ2500 = 35,
    OR_TYPEID_PANASONIC_FZ2000 = OR_TYPEID_PANASONIC_FZ2500,
    OR_TYPEID_PANASONIC_GX8 = 36,
    OR_TYPEID_PANASONIC_ZS100 = 37,
    OR_TYPEID_PANASONIC_TX1 = OR_TYPEID_PANASONIC_ZS100,
    OR_TYPEID_PANASONIC_TZ100 = OR_TYPEID_PANASONIC_ZS100,
    OR_TYPEID_PANASONIC_TZ110 = OR_TYPEID_PANASONIC_ZS100,
    OR_TYPEID_PANASONIC_GX80 = 38,
    OR_TYPEID_PANASONIC_GX85 = OR_TYPEID_PANASONIC_GX80,
    OR_TYPEID_PANASONIC_GH5 = 39,
    OR_TYPEID_PANASONIC_GX850 = 40,
    OR_TYPEID_PANASONIC_FZ80 = 41,
    OR_TYPEID_PANASONIC_FZ82 = OR_TYPEID_PANASONIC_FZ80,
    OR_TYPEID_PANASONIC_FZ330 = 42,
    OR_TYPEID_PANASONIC_TZ70 = 43,
    OR_TYPEID_PANASONIC_ZS60 = 44,
    OR_TYPEID_PANASONIC_TZ80 = OR_TYPEID_PANASONIC_ZS60,

    OR_TYPEID_PANASONIC_GF7 = 46,
    OR_TYPEID_PANASONIC_CM1 = 47,
    OR_TYPEID_PANASONIC_GX9 = 48,
    OR_TYPEID_PANASONIC_GX800 = 49,

    OR_TYPEID_PANASONIC_G9 = 52,
    OR_TYPEID_PANASONIC_DC_FZ45 = 53, // Not the DMC FZ45
    OR_TYPEID_PANASONIC_GH5S = 54,
    OR_TYPEID_PANASONIC_LX1 = 55,
    OR_TYPEID_PANASONIC_FZ150 = 56,
    OR_TYPEID_PANASONIC_FZ35 = 57,
    OR_TYPEID_PANASONIC_ZS200 = 58,
    OR_TYPEID_PANASONIC_TX2 = OR_TYPEID_PANASONIC_ZS200,
    OR_TYPEID_PANASONIC_TZ202 = OR_TYPEID_PANASONIC_ZS200,
    OR_TYPEID_PANASONIC_GX7MK2 = 59,
    OR_TYPEID_PANASONIC_LX100M2 = 60,
    OR_TYPEID_PANASONIC_DMC_FZ40 = 61,
    OR_TYPEID_PANASONIC_DMC_FZ45 = OR_TYPEID_PANASONIC_DMC_FZ40, // Not the DC FZ45
    OR_TYPEID_PANASONIC_DC_S1 = 62,
    OR_TYPEID_PANASONIC_DC_S1R = 63,
    OR_TYPEID_PANASONIC_DC_G95 = 64,
    OR_TYPEID_PANASONIC_DMC_FZ1000 = 65,
    OR_TYPEID_PANASONIC_DC_FZ1000M2 = 66,
    OR_TYPEID_PANASONIC_DC_ZS80 = 67,
    OR_TYPEID_PANASONIC_DC_TZ95 = OR_TYPEID_PANASONIC_DC_ZS80,
    OR_TYPEID_PANASONIC_GF10 = 68,
    OR_TYPEID_PANASONIC_GX880 = OR_TYPEID_PANASONIC_GF10,
    OR_TYPEID_PANASONIC_DC_G99 = 69,
    OR_TYPEID_PANASONIC_DC_G91 = OR_TYPEID_PANASONIC_DC_G99,
    OR_TYPEID_PANASONIC_DC_S1H = 70,
    OR_TYPEID_PANASONIC_DC_G100 = 71,
    OR_TYPEID_PANASONIC_DC_S5 = 72,
    OR_TYPEID_PANASONIC_GH5M2 = 73,
    OR_TYPEID_PANASONIC_GH6 = 74,
    _OR_TYPEID_PANASONIC_LAST
};

/** @brief Fujifilm type IDs */
enum _OR_TYPEID_VENDOR_FUJIFILM {
    OR_TYPEID_FUJIFILM_UNKNOWN = 0,
    OR_TYPEID_FUJIFILM_F700 = 1,
    OR_TYPEID_FUJIFILM_E900 = 2,
    OR_TYPEID_FUJIFILM_S2PRO = 3,
    OR_TYPEID_FUJIFILM_S3PRO = 4,
    OR_TYPEID_FUJIFILM_S5PRO = 5,
    OR_TYPEID_FUJIFILM_F810 = 6,
    OR_TYPEID_FUJIFILM_S5000 = 7,
    OR_TYPEID_FUJIFILM_S5600 = 8,
    OR_TYPEID_FUJIFILM_S9500 = 9,
    OR_TYPEID_FUJIFILM_S6500FD = 10,
    OR_TYPEID_FUJIFILM_HS10 = 11,
    OR_TYPEID_FUJIFILM_HS30EXR = 12,
    OR_TYPEID_FUJIFILM_HS33EXR = OR_TYPEID_FUJIFILM_HS30EXR,
    OR_TYPEID_FUJIFILM_S200EXR = 13,
    OR_TYPEID_FUJIFILM_X100 = 14,
    OR_TYPEID_FUJIFILM_X100S = 15,
    OR_TYPEID_FUJIFILM_X100T = 16,
    OR_TYPEID_FUJIFILM_X100F = 17,
    OR_TYPEID_FUJIFILM_X10 = 18,
    OR_TYPEID_FUJIFILM_X20 = 19,
    OR_TYPEID_FUJIFILM_X30 = 20,
    OR_TYPEID_FUJIFILM_X70 = 21,
    OR_TYPEID_FUJIFILM_XPRO1 = 22,
    OR_TYPEID_FUJIFILM_XPRO2 = 23,
    OR_TYPEID_FUJIFILM_XS1 = 24,
    OR_TYPEID_FUJIFILM_XE1 = 25,
    OR_TYPEID_FUJIFILM_XE2 = 26,
    OR_TYPEID_FUJIFILM_XE2S = 27,
    OR_TYPEID_FUJIFILM_XE3 = 28,
    OR_TYPEID_FUJIFILM_XF1 = 29,
    OR_TYPEID_FUJIFILM_XM1 = 30,
    OR_TYPEID_FUJIFILM_XT1 = 31,
    OR_TYPEID_FUJIFILM_XT10 = 32,
    OR_TYPEID_FUJIFILM_XT100 = 33,
    OR_TYPEID_FUJIFILM_XT2 = 34,
    OR_TYPEID_FUJIFILM_XT20 = 35,
    OR_TYPEID_FUJIFILM_XT3 = 36,
    OR_TYPEID_FUJIFILM_XA1 = 37,
    OR_TYPEID_FUJIFILM_XA2 = 38,
    OR_TYPEID_FUJIFILM_XA3 = 39,
    OR_TYPEID_FUJIFILM_XA5 = 40,
    OR_TYPEID_FUJIFILM_XQ1 = 41,
    OR_TYPEID_FUJIFILM_XQ2 = 42,
    OR_TYPEID_FUJIFILM_XH1 = 43,
    OR_TYPEID_FUJIFILM_GFX50S = 44,
    OR_TYPEID_FUJIFILM_GFX50R = 45,
    OR_TYPEID_FUJIFILM_XF10 = 46,
    OR_TYPEID_FUJIFILM_XT30 = 47,
    OR_TYPEID_FUJIFILM_GFX100 = 48,
    OR_TYPEID_FUJIFILM_XA7 = 49,
    OR_TYPEID_FUJIFILM_XPRO3 = 50,
    OR_TYPEID_FUJIFILM_XT200 = 51,
    OR_TYPEID_FUJIFILM_X100V = 52,
    OR_TYPEID_FUJIFILM_XT4 = 53,
    OR_TYPEID_FUJIFILM_F550EXR = 54,
    OR_TYPEID_FUJIFILM_S100FS = 55,
    OR_TYPEID_FUJIFILM_XS10 = 56,
    _OR_TYPEID_FUJIFILM_LAST
};

enum _OR_TYPEID_VENDOR_BLACKMAGIC {
    OR_TYPEID_BLACKMAGIC_UNKNOWN = 0,
    OR_TYPEID_BLACKMAGIC_POCKET_CINEMA = 1,
    _OR_TYPEID_BLACKMAGIC_LAST
};

enum _OR_TYPEID_VENDOR_XIAOYI {
    OR_TYPEID_XIAOYI_UNKNOWN = 0,
    OR_TYPEID_XIAOYI_M1 = 1,
    OR_TYPEID_XIAOYI_YDXJ_2 = 2,
    OR_TYPEID_XIAOYI_YIAC_3 = 3,
    _OR_TYPEID_XIAOYI_LAST
};

enum _OR_TYPEID_VENDOR_APPLE {
    OR_TYPEID_APPLE_UNKNOWN = 0,
    OR_TYPEID_APPLE_IPHONE_6SPLUS = 1,
    OR_TYPEID_APPLE_IPHONE_7PLUS = 2,
    OR_TYPEID_APPLE_IPHONE_SE = 3,
    OR_TYPEID_APPLE_IPHONE_8 = 4,
    OR_TYPEID_APPLE_IPHONE_XS = 5,
    _OR_TYPEID_APPLE_LAST
};

enum _OR_TYPEID_VENDOR_SIGMA {
    OR_TYPEID_SIGMA_UNKNOWN = 0,
    OR_TYPEID_SIGMA_FP = 1,
    _OR_TYPEID_SIGMA_LAST
};

enum _OR_TYPEID_VENDOR_GOPRO {
    OR_TYPEID_GOPRO_UNKNOWN = 0,
    OR_TYPEID_GOPRO_HERO5_BLACK = 1,
    OR_TYPEID_GOPRO_HERO6_BLACK = 2,
    OR_TYPEID_GOPRO_HERO7_BLACK = 3,
    OR_TYPEID_GOPRO_HERO8_BLACK = 4,

    _OR_TYPEID_GOPRO_LAST
};

enum _OR_TYPEID_VENDOR_HASSELBLAD {
    OR_TYPEID_HASSELBLAD_UNKNOWN = 0,
    OR_TYPEID_HASSELBLAD_LUNAR = 1,
    OR_TYPEID_HASSELBLAD_L1D_20C = 2,

    _OR_TYPEID_HASSELBLAD_LAST
};

enum _OR_TYPEID_VENDOR_ZEISS {
    OR_TYPEID_ZEISS_UNKNOWN = 0,
    OR_TYPEID_ZEISS_ZX1 = 1,

    _OR_TYPEID_ZEISS_LAST
};

#ifdef __cplusplus
}
#endif

/** @} */
#endif
