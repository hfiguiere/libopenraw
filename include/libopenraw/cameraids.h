/*
 * libopenraw - cameraids.h
 *
 * Copyright (C) 2012-2013 Hubert Figuière
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
 * @brief the libopenraw public consts types
 * @author Hubert Figuière <hub@figuiere.net>
 */

#ifndef __LIBOPENRAW_CAMERAIDS_H__
#define __LIBOPENRAW_CAMERAIDS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	
    /** The vendor ID: the high order 16-bits of the %or_rawfile_typeid
     */
    enum {
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
        /* not really a camera vendor. For the converter. */
        OR_TYPEID_VENDOR_ADOBE = 13,
        OR_TYPEID_VENDOR_FUJIFILM = 14,
	
        _OR_TYPEID_VENDOR_LAST
    };
    
    enum {
        OR_TYPEID_UNKNOWN = 0
    };
    
    enum {
        OR_TYPEID_ADOBE_UNKNOWN = 0,
        OR_TYPEID_ADOBE_DNG_GENERIC = 1,
        _OR_TYPEID_ADOBE_LAST
    };
    
    /** Canon type IDs */
    enum {
        OR_TYPEID_CANON_UNKNOWN = 0,
        OR_TYPEID_CANON_20D     = 1,
        OR_TYPEID_CANON_30D     = 2,
        OR_TYPEID_CANON_40D     = 3,
        OR_TYPEID_CANON_350D    = 4,
        OR_TYPEID_CANON_REBEL_XT= OR_TYPEID_CANON_350D,
        OR_TYPEID_CANON_400D    = 5,
        OR_TYPEID_CANON_450D    = 6,
        OR_TYPEID_CANON_5D      = 7,
        OR_TYPEID_CANON_1D      = 8,
        OR_TYPEID_CANON_1DMKII  = 9,
        OR_TYPEID_CANON_1DMKIII = 10,
        OR_TYPEID_CANON_1DS     = 11,
        OR_TYPEID_CANON_1DSMKII = 12,
        OR_TYPEID_CANON_1DSMKIII= 13,
        OR_TYPEID_CANON_300D    = 14,
        OR_TYPEID_CANON_D30     = 15,
        OR_TYPEID_CANON_D60     = 16,
        OR_TYPEID_CANON_10D     = 17,
        OR_TYPEID_CANON_PRO1    = 18,
        OR_TYPEID_CANON_G1      = 19,
        OR_TYPEID_CANON_G2      = 20,
        OR_TYPEID_CANON_G3      = 21,
        OR_TYPEID_CANON_G5      = 22,
        OR_TYPEID_CANON_G6      = 23,
        OR_TYPEID_CANON_G7      = 24,
        OR_TYPEID_CANON_G9      = 25,
        OR_TYPEID_CANON_A610    = 26,
        OR_TYPEID_CANON_20DA    = 27,
        OR_TYPEID_CANON_7D      = 28,
        OR_TYPEID_CANON_G11     = 29,
        OR_TYPEID_CANON_1DMKIV  = 30,
        OR_TYPEID_CANON_500D    = 31,
        OR_TYPEID_CANON_5DMKII  = 32,
        OR_TYPEID_CANON_550D    = 33,
        OR_TYPEID_CANON_REBEL_T2I= OR_TYPEID_CANON_550D,
        OR_TYPEID_CANON_1000D   = 34,
        OR_TYPEID_CANON_REBEL_XS= OR_TYPEID_CANON_1000D,
        OR_TYPEID_CANON_G10     = 35,
        OR_TYPEID_CANON_50D     = 36,
        OR_TYPEID_CANON_60D     = 36,
        OR_TYPEID_CANON_S90     = 37,
        OR_TYPEID_CANON_G12     = 38,
        OR_TYPEID_CANON_S95     = 39,
        OR_TYPEID_CANON_600D    = 40,
        OR_TYPEID_CANON_REBEL_T3I= OR_TYPEID_CANON_600D,
        OR_TYPEID_CANON_1100D   = 41,
        OR_TYPEID_CANON_REBEL_T3= OR_TYPEID_CANON_1100D,
        OR_TYPEID_CANON_G1X     = 42,
        OR_TYPEID_CANON_S100    = 43,
        OR_TYPEID_CANON_5DMKIII = 44,
        OR_TYPEID_CANON_1DX     = 45,
        OR_TYPEID_CANON_60Da    = 46,
        OR_TYPEID_CANON_650D    = 47,
        OR_TYPEID_CANON_REBEL_T4I= OR_TYPEID_CANON_650D,
        OR_TYPEID_CANON_G15     = 48,
        OR_TYPEID_CANON_6D      = 49,
        OR_TYPEID_CANON_EOS_M   = 50,
        OR_TYPEID_CANON_SX50_HS = 51,
        OR_TYPEID_CANON_S110    = 52,
        OR_TYPEID_CANON_100D    = 53,
        OR_TYPEID_CANON_REBEL_SL1= OR_TYPEID_CANON_100D,
        OR_TYPEID_CANON_700D    = 54,
        OR_TYPEID_CANON_REBEL_T5I= OR_TYPEID_CANON_700D,
        OR_TYPEID_CANON_G16     = 55,
        OR_TYPEID_CANON_70D     = 56,
        _OR_TYPEID_CANON_LAST
    };


    /** Nikon type IDs */
    enum {
        OR_TYPEID_NIKON_UNKNOWN = 0,
        OR_TYPEID_NIKON_COOLPIX_5700 ,
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
        _OR_TYPEID_NIKON_LAST
    };

    /** Leica type IDs */
    enum {
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
        _OR_TYPEID_LEICA_LAST
    };

    /** Pentax type IDs */
    enum {
        OR_TYPEID_PENTAX_UNKNOWN = 0,
        OR_TYPEID_PENTAX_K10D_PEF,
        OR_TYPEID_PENTAX_K10D_DNG,
        OR_TYPEID_PENTAX_IST_D,
        OR_TYPEID_PENTAX_IST_DL,
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
        _OR_TYPEID_PENTAX_LAST
    };

    /** Epson type IDs */
    enum {
        OR_TYPEID_EPSON_UNKNOWN = 0,
        OR_TYPEID_EPSON_RD1 = 1,
        OR_TYPEID_EPSON_RD1S = 2,
        _OR_TYPEID_EPSON_LAST
    };

    /** Minolta type IDs */
    enum {
        OR_TYPEID_MINOLTA_UNKNOWN = 0,
        OR_TYPEID_MINOLTA_A1,
        OR_TYPEID_MINOLTA_A2,
        OR_TYPEID_MINOLTA_DIMAGE5,
        OR_TYPEID_MINOLTA_DIMAGE7,
        OR_TYPEID_MINOLTA_DIMAGE7I,
        OR_TYPEID_MINOLTA_DIMAGE7HI,
        OR_TYPEID_MINOLTA_MAXXUM_5D,
        OR_TYPEID_MINOLTA_MAXXUM_7D,
        OR_TYPEID_MINOLTA_A200
    };

    enum {
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
        OR_TYPEID_OLYMPUS_SP510,
        OR_TYPEID_OLYMPUS_SP550,
        OR_TYPEID_OLYMPUS_SP500,
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
        _OR_TYPEID_OLYMPUS_LAST
    };

    enum {
        OR_TYPEID_SAMSUNG_UNKNOWN = 0,
        OR_TYPEID_SAMSUNG_GX10,
        OR_TYPEID_SAMSUNG_PRO815
    };


    enum {
        OR_TYPEID_RICOH_UNKNOWN = 0,
        OR_TYPEID_RICOH_GR2 = 1,
        OR_TYPEID_RICOH_GXR = 2,
        OR_TYPEID_RICOH_GXR_A16 = 3,
        OR_TYPEID_RICOH_GR = 4, /* 2013 Ricoh GR-D */
        _OR_TYPEID_RICOH_LAST
    };

    enum {
        OR_TYPEID_SONY_UNKNOWN = 0,
        OR_TYPEID_SONY_A100,
        OR_TYPEID_SONY_A200,
        OR_TYPEID_SONY_A700,
        OR_TYPEID_SONY_A550,
        OR_TYPEID_SONY_A380,
        OR_TYPEID_SONY_A390,
        OR_TYPEID_SONY_SLTA55,
        OR_TYPEID_SONY_SLTA77,
        OR_TYPEID_SONY_NEX3,
        OR_TYPEID_SONY_NEX5,
        OR_TYPEID_SONY_NEX5N,
        OR_TYPEID_SONY_NEXC3,
        OR_TYPEID_SONY_SLTA65,
        OR_TYPEID_SONY_NEX7,
        OR_TYPEID_SONY_A330,
        OR_TYPEID_SONY_A350,
        OR_TYPEID_SONY_A450,
        OR_TYPEID_SONY_A580,
        OR_TYPEID_SONY_A850,
        OR_TYPEID_SONY_A900,
        OR_TYPEID_SONY_SLTA35,
        OR_TYPEID_SONY_SLTA33,
        OR_TYPEID_SONY_A560,
        OR_TYPEID_SONY_SLTA99,
        OR_TYPEID_SONY_RX100,
        OR_TYPEID_SONY_RX1,
        OR_TYPEID_SONY_NEX6,
        OR_TYPEID_SONY_SLTA57,
        OR_TYPEID_SONY_NEXF3,
        OR_TYPEID_SONY_NEX5R,
        OR_TYPEID_SONY_RX100M2,
        OR_TYPEID_SONY_RX1R,
        _OR_TYPEID_SONY_LAST
    };

    enum {
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
        _OR_TYPEID_PANASONIC_LAST
    };

    enum {
        OR_TYPEID_FUJIFILM_UNKNOWN = 0,
        OR_TYPEID_FUJIFILM_X100 = 1,
        OR_TYPEID_FUJIFILM_F700 = 2,
        OR_TYPEID_FUJIFILM_E900 = 3,
        OR_TYPEID_FUJIFILM_S2PRO = 4,
        OR_TYPEID_FUJIFILM_S3PRO = 5,
        OR_TYPEID_FUJIFILM_S5PRO = 6,
        OR_TYPEID_FUJIFILM_F810 = 7,
        OR_TYPEID_FUJIFILM_S5600 = 8,
        OR_TYPEID_FUJIFILM_S9500 = 9,
        OR_TYPEID_FUJIFILM_S6500FD = 10,
        OR_TYPEID_FUJIFILM_HS10 = 11,
        OR_TYPEID_FUJIFILM_X10 = 12,
        OR_TYPEID_FUJIFILM_XPRO1 = 13,
        OR_TYPEID_FUJIFILM_XS1 = 14,
        OR_TYPEID_FUJIFILM_S200EXR = 15,
        OR_TYPEID_FUJIFILM_XE1 = 16,
        OR_TYPEID_FUJIFILM_XF1 = 17,
        OR_TYPEID_FUJIFILM_X100S = 18,
        OR_TYPEID_FUJIFILM_X20 = 19,
        OR_TYPEID_FUJIFILM_XM1 = 20,
        _OR_TYPEID_FUJIFILM_LAST
    };

#ifdef __cplusplus
}
#endif

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
