// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - camera_ids.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

//! Define all the file IDs.

/// The vendor ID is `TypeId::0`
#[allow(unused)]
pub mod vendor {
    pub const NONE: u16 = 0;
    pub const CANON: u16 = 1;
    pub const NIKON: u16 = 2;
    /// Leica branded camera, including plain Panasonic rebadged.
    pub const LEICA: u16 = 3;
    /// Pentax merged with Ricoh so depending of the era the IDs might be
    /// with Ricoh.
    pub const PENTAX: u16 = 4;
    pub const EPSON: u16 = 5;
    pub const MINOLTA: u16 = 6;
    pub const OLYMPUS: u16 = 7;
    pub const SONY: u16 = 8;
    pub const SAMSUNG: u16 = 9;
    pub const RICOH: u16 = 10;
    pub const PANASONIC: u16 = 11;
    pub const MAMIYA: u16 = 12;
    /// Generic DNG files.
    pub const ADOBE: u16 = 13;
    pub const FUJIFILM: u16 = 14;
    pub const BLACKMAGIC: u16 = 15;
    pub const XIAOYI: u16 = 16;
    pub const APPLE: u16 = 17;
    pub const SIGMA: u16 = 18;
    pub const GOPRO: u16 = 19;
    pub const HASSELBLAD: u16 = 20;
    pub const ZEISS: u16 = 21;
    pub const DJI: u16 = 22;
    pub const NOKIA: u16 = 23;
    /// JPEG isn't a vendor.
    pub const JPEG: u16 = 1000;
}

pub mod generic {
    pub const UNKNOWN: u16 = 0;
}

/// Adobe type IDs
pub mod adobe {
    /// Generic DNG file.
    pub const DNG_GENERIC: u16 = 1;
}

/// Canon type IDs
#[allow(unused)]
pub mod canon {
    pub const UNKNOWN: u16 = 0;
    pub const EOS_20D: u16 = 1;
    pub const EOS_30D: u16 = 2;
    pub const EOS_40D: u16 = 3;
    pub const EOS_350D: u16 = 4;
    #[allow(unused)]
    pub const REBEL_XT: u16 = EOS_350D;
    #[allow(unused)]
    pub const KISS_DIGITAL_N: u16 = EOS_350D;
    pub const EOS_400D: u16 = 5;
    #[allow(unused)]
    pub const REBEL_XTI: u16 = EOS_400D;
    #[allow(unused)]
    pub const KISS_DIGITAL_X: u16 = EOS_400D;
    pub const EOS_450D: u16 = 6;
    #[allow(unused)]
    pub const REBEL_XSI: u16 = EOS_450D;
    #[allow(unused)]
    pub const KISS_X2: u16 = EOS_450D;
    pub const EOS_5D: u16 = 7;
    pub const EOS_1D: u16 = 8;
    pub const EOS_1DMKII: u16 = 9;
    pub const EOS_1DMKIII: u16 = 10;
    pub const EOS_1DS: u16 = 11;
    pub const EOS_1DSMKII: u16 = 12;
    pub const EOS_1DSMKIII: u16 = 13;
    pub const EOS_300D: u16 = 14;
    pub const DIGITAL_REBEL: u16 = EOS_300D;
    pub const EOS_D30: u16 = 15;
    pub const EOS_D60: u16 = 16;
    pub const EOS_10D: u16 = 17;
    pub const PRO1: u16 = 18;
    pub const G1: u16 = 19;
    pub const G2: u16 = 20;
    pub const G3: u16 = 21;
    pub const G5: u16 = 22;
    pub const G6: u16 = 23;
    pub const G7: u16 = 24;
    pub const G9: u16 = 25;
    pub const A610: u16 = 26;
    pub const EOS_20DA: u16 = 27;
    pub const EOS_7D: u16 = 28;
    pub const G11: u16 = 29;
    pub const EOS_1DMKIV: u16 = 30;
    pub const EOS_500D: u16 = 31;
    #[allow(unused)]
    pub const REBEL_T1I: u16 = EOS_500D;
    #[allow(unused)]
    pub const KISS_X3: u16 = EOS_500D;
    pub const EOS_5DMKII: u16 = 32;
    pub const EOS_550D: u16 = 33;
    #[allow(unused)]
    pub const REBEL_T2I: u16 = EOS_550D;
    #[allow(unused)]
    pub const KISS_X4: u16 = EOS_550D;
    pub const EOS_1000D: u16 = 34;
    #[allow(unused)]
    pub const REBEL_XS: u16 = EOS_1000D;
    #[allow(unused)]
    pub const KISS_F: u16 = EOS_1000D;
    pub const G10: u16 = 35;
    pub const EOS_50D: u16 = 36;
    pub const EOS_60D: u16 = 36;
    pub const S90: u16 = 37;
    pub const G12: u16 = 38;
    pub const S95: u16 = 39;
    pub const EOS_600D: u16 = 40;
    #[allow(unused)]
    pub const REBEL_T3I: u16 = EOS_600D;
    #[allow(unused)]
    pub const KISS_X5: u16 = EOS_600D;
    pub const EOS_1100D: u16 = 41;
    #[allow(unused)]
    pub const REBEL_T3: u16 = EOS_1100D;
    #[allow(unused)]
    pub const KISS_X50: u16 = EOS_1100D;
    pub const G1X: u16 = 42;
    pub const S100: u16 = 43;
    pub const EOS_5DMKIII: u16 = 44;
    pub const EOS_1DX: u16 = 45;
    pub const EOS_60DA: u16 = 46;
    pub const EOS_650D: u16 = 47;
    #[allow(unused)]
    pub const REBEL_T4I: u16 = EOS_650D;
    #[allow(unused)]
    pub const KISS_X6I: u16 = EOS_650D;
    pub const G15: u16 = 48;
    pub const EOS_6D: u16 = 49;
    pub const EOS_M: u16 = 50;
    pub const SX50_HS: u16 = 51;
    pub const S110: u16 = 52;
    pub const EOS_100D: u16 = 53;
    #[allow(unused)]
    pub const REBEL_SL1: u16 = EOS_100D;
    #[allow(unused)]
    pub const KISS_X7: u16 = EOS_100D;
    pub const EOS_700D: u16 = 54;
    #[allow(unused)]
    pub const REBEL_T5I: u16 = EOS_700D;
    #[allow(unused)]
    pub const KISS_X7I: u16 = EOS_700D;
    pub const G16: u16 = 55;
    pub const EOS_70D: u16 = 56;
    pub const EOS_7DMKII: u16 = 57;
    pub const G7X: u16 = 58;
    pub const G1XMKII: u16 = 59;
    pub const EOS_750D: u16 = 60;
    #[allow(unused)]
    pub const REBEL_T6I: u16 = EOS_750D;
    #[allow(unused)]
    pub const KISS_X8I: u16 = EOS_750D;
    pub const EOS_760D: u16 = 61;
    #[allow(unused)]
    pub const REBEL_T6S: u16 = EOS_760D;
    #[allow(unused)]
    pub const EOS_8000D: u16 = EOS_760D;
    pub const EOS_5DS_R: u16 = 62;
    pub const G3X: u16 = 63;
    pub const G9XMKII: u16 = 64;
    pub const EOS_5DMKIV: u16 = 65;
    pub const EOS_M5: u16 = 66;
    pub const G5X: u16 = 67;
    pub const G7XMKII: u16 = 68;
    pub const EOS_1300D: u16 = 69;
    #[allow(unused)]
    pub const REBEL_T6: u16 = EOS_1300D;
    #[allow(unused)]
    pub const KISS_X80: u16 = EOS_1300D;
    pub const EOS_M3: u16 = 70;
    pub const EOS_1DXMKII: u16 = 71;
    pub const EOS_80D: u16 = 72;
    pub const EOS_1200D: u16 = 73;
    #[allow(unused)]
    pub const REBEL_T5: u16 = EOS_1200D;
    #[allow(unused)]
    pub const KISS_X70: u16 = EOS_1200D;
    pub const G9X: u16 = 74;
    pub const EOS_M10: u16 = 75;
    pub const EOS_800D: u16 = 76;
    #[allow(unused)]
    pub const REBEL_T7I: u16 = EOS_800D;
    #[allow(unused)]
    pub const KISS_X9I: u16 = EOS_800D;
    pub const EOS_77D: u16 = 77;
    #[allow(unused)]
    pub const EOS_9000D: u16 = EOS_77D;
    pub const EOS_M6: u16 = 78;
    pub const EOS_M100: u16 = 79;
    pub const EOS_6DMKII: u16 = 80;
    pub const EOS_200D: u16 = 81;
    #[allow(unused)]
    pub const REBEL_SL2: u16 = EOS_200D;
    #[allow(unused)]
    pub const KISS_X9: u16 = EOS_200D;
    pub const G1XMKIII: u16 = 82;
    pub const EOS_5DS: u16 = 83;
    pub const EOS_M50: u16 = 84;
    #[allow(unused)]
    pub const KISS_M: u16 = EOS_M50;
    pub const SX1_IS: u16 = 85;
    pub const S120: u16 = 86;
    pub const SX60_HS: u16 = 87;
    pub const EOS_2000D: u16 = 88;
    #[allow(unused)]
    pub const REBEL_T7: u16 = EOS_2000D;
    #[allow(unused)]
    pub const KISS_X90: u16 = EOS_2000D;
    pub const EOS_R: u16 = 89;
    pub const EOS_3000D: u16 = 90;
    #[allow(unused)]
    pub const T100: u16 = EOS_3000D;
    #[allow(unused)]
    pub const EOS_4000D: u16 = EOS_3000D;
    pub const EOS_1DMKIIN: u16 = 91;
    pub const SX70_HS: u16 = 92;
    pub const EOS_RP: u16 = 93;
    pub const EOS_250D: u16 = 94;
    #[allow(unused)]
    pub const REBEL_SL3: u16 = EOS_250D;
    #[allow(unused)]
    pub const KISS_X10: u16 = EOS_250D;
    #[allow(unused)]
    pub const EOS_200DMKII: u16 = EOS_250D;
    pub const G7XMKIII: u16 = 95;
    pub const G5XMKII: u16 = 96;
    pub const EOS_M6MKII: u16 = 97;
    pub const EOS_90D: u16 = 98;
    pub const EOS_M200: u16 = 99;
    pub const EOS_1DXMKIII: u16 = 100;
    pub const EOS_R5: u16 = 101;
    pub const EOS_R6: u16 = 102;
    pub const S30: u16 = 103;
    pub const S40: u16 = 104;
    pub const S45: u16 = 105;
    pub const S50: u16 = 106;
    pub const S60: u16 = 107;
    pub const S70: u16 = 108;
    pub const S100V: u16 = 109;
    pub const PRO70: u16 = 110;
    pub const PRO90: u16 = 111;
    pub const EOS_850D: u16 = 112;
    #[allow(unused)]
    pub const REBEL_T8I: u16 = EOS_850D;
    #[allow(unused)]
    pub const KISS_X10I: u16 = EOS_850D;
    pub const EOS_M2: u16 = 113;
    pub const EOS_M50MKII: u16 = 114;
    #[allow(unused)]
    pub const KISS_M2: u16 = EOS_M50MKII;
    pub const EOS_R3: u16 = 115;
    pub const EOS_R7: u16 = 116;
    pub const EOS_R10: u16 = 117;
    pub const EOS_R6MKII: u16 = 118;
    pub const EOS_R8: u16 = 119;
    pub const EOS_R50: u16 = 120;
    pub const EOS_R100: u16 = 121;
}

/// Nikon type IDs
pub mod nikon {
    pub const UNKNOWN: u16 = 0;
    pub const E5700: u16 = 1;
    pub const D1: u16 = 2;
    pub const D1X: u16 = 3;
    pub const D100: u16 = 4;
    pub const D2H: u16 = 5;
    pub const D2X: u16 = 6;
    pub const D200: u16 = 7;
    pub const D3: u16 = 8;
    pub const D300: u16 = 9;
    pub const D40: u16 = 10;
    pub const D40X: u16 = 11;
    pub const D50: u16 = 12;
    pub const D70: u16 = 13;
    pub const D70S: u16 = 14;
    pub const D80: u16 = 15;
    pub const D3000: u16 = 16;
    pub const COOLPIX_P6000: u16 = 17;
    pub const COOLPIX_P7000: u16 = 18;
    pub const D7000: u16 = 19;
    pub const D3100: u16 = 20;
    pub const NIKON1_J1: u16 = 21;
    pub const NIKON1_V1: u16 = 22;
    pub const COOLPIX_P7100: u16 = 23;
    pub const D5000: u16 = 24;
    pub const D5100: u16 = 25;
    pub const D4: u16 = 26;
    pub const D3S: u16 = 27;
    pub const D3X: u16 = 28;
    pub const D300S: u16 = 29;
    pub const D3200: u16 = 30;
    pub const D700: u16 = 31;
    pub const D800: u16 = 32;
    pub const D800E: u16 = 33;
    pub const D90: u16 = 34;
    pub const D600: u16 = 35;
    pub const COOLPIX_P7700: u16 = 36;
    pub const NIKON1_V2: u16 = 37;
    pub const D5200: u16 = 38;
    pub const D7100: u16 = 39;
    pub const COOLPIX_A: u16 = 40;
    pub const NIKON1_J2: u16 = 41;
    pub const NIKON1_J3: u16 = 42;
    pub const NIKON1_S1: u16 = 43;
    pub const D60: u16 = 44;
    pub const DF: u16 = 45;
    pub const E5400: u16 = 46;
    pub const E8400: u16 = 47;
    pub const D4S: u16 = 48;
    pub const D610: u16 = 49;
    pub const D750: u16 = 50;
    pub const NIKON1_J5: u16 = 51;
    pub const NIKON1_V3: u16 = 52;
    pub const D7200: u16 = 53;
    pub const D5300: u16 = 54;
    pub const D5500: u16 = 55;
    pub const D3300: u16 = 56;
    pub const D810: u16 = 57;
    pub const D5600: u16 = 58;
    pub const D3400: u16 = 59;
    pub const D5: u16 = 60;
    pub const D500: u16 = 61;
    pub const NIKON1_AW1: u16 = 62;
    pub const NIKON1_S2: u16 = 63;
    pub const NIKON1_J4: u16 = 64;
    pub const COOLPIX_B700: u16 = 65;
    pub const COOLPIX_P330: u16 = 66;
    pub const COOLPIX_P340: u16 = 67;
    pub const Z6: u16 = 68;
    pub const Z7: u16 = 69;
    pub const COOLPIX_P1000: u16 = 70;
    pub const E8800: u16 = 71;
    pub const D3500: u16 = 72;
    pub const D2HS: u16 = 73;
    pub const D2XS: u16 = 74;
    pub const COOLPIX_A1000: u16 = 75;
    pub const D780: u16 = 76;
    pub const Z50: u16 = 77;
    pub const COOLPIX_P950: u16 = 78;
    pub const D6: u16 = 79;
    pub const Z5: u16 = 80;
    pub const Z6_2: u16 = 81;
    pub const Z7_2: u16 = 82;
    pub const ZFC: u16 = 83;
    pub const Z9: u16 = 84;
    pub const D1H: u16 = 85;
    pub const D7500: u16 = 86;
    pub const D850: u16 = 87;
    pub const COOLPIX_P7800: u16 = 88;
    pub const Z30: u16 = 89;
    pub const Z8: u16 = 90;
}

/// Leica type IDs
pub mod leica {
    /* DNG */
    pub const DMR: u16 = 1;
    pub const M8: u16 = 2;
    pub const X1: u16 = 3;
    /* Panasonic RAW */
    pub const DIGILUX2: u16 = 4;
    pub const DLUX_3: u16 = 5;
    pub const VLUX_1: u16 = 6;
    /* DNG */
    pub const M9: u16 = 7;
    pub const S2: u16 = 8;
    pub const M_MONOCHROM: u16 = 9;
    pub const X2: u16 = 10;
    pub const M_TYP240: u16 = 11;
    pub const X_VARIO: u16 = 12;
    pub const T_TYP701: u16 = 13;
    pub const Q_TYP116: u16 = 14;
    pub const X_TYP113: u16 = 15;
    pub const M10: u16 = 16;
    pub const SL_TYP601: u16 = 17;
    /* RWL (Panasonic RAW) */
    pub const DLUX_TYP109: u16 = 18;
    pub const VLUX_4: u16 = 19;
    /* DNG */
    pub const CL: u16 = 20;
    /* RWL (Panasonic RAW) */
    pub const VLUX_TYP114: u16 = 21;
    pub const CLUX: u16 = 22;
    /* DNG */
    pub const M10P: u16 = 23;
    pub const M10D: u16 = 24;
    pub const TL2: u16 = 25;
    /* RWL */
    pub const DLUX_5: u16 = 26;
    pub const DLUX_7: u16 = 27;
    pub const C_TYP112: u16 = 28;
    pub const VLUX_5: u16 = 29;
    /* DNG */
    pub const Q2: u16 = 30;
    pub const SL2: u16 = 31;
    pub const M10_MONOCHROM: u16 = 32;
    pub const M10R: u16 = 33;
    pub const M_MONOCHROM_TYP246: u16 = 34;
    /* RWL */
    pub const DLUX_4: u16 = 35;
    /* DNG */
    pub const SL2S: u16 = 36;
    pub const Q2_MONOCHROM: u16 = 37;
    pub const M11: u16 = 38;
    /* RW2 */
    pub const DIGILUX3: u16 = 39;
    /* RWL */
    pub const DLUX_6: u16 = 40;
    /* DNG */
    pub const M11_MONOCHROM: u16 = 41;
    pub const Q3: u16 = 42;
    pub const SL3: u16 = 43;
}

/// Pentax type IDs
pub mod pentax {
    pub const UNKNOWN: u16 = 0;
    pub const K10D_PEF: u16 = 1;
    pub const K10D_DNG: u16 = 2;
    pub const IST_D_PEF: u16 = 3;
    pub const IST_DL_PEF: u16 = 4;
    pub const K100D_PEF: u16 = 5;
    pub const K100D_SUPER_PEF: u16 = 6;
    pub const K20D_PEF: u16 = 7;
    pub const KR_PEF: u16 = 8;
    pub const KX_PEF: u16 = 9;
    pub const K5_PEF: u16 = 10;
    pub const K7_PEF: u16 = 11;
    pub const PENTAX_645D_PEF: u16 = 12;
    pub const PENTAX_645D_DNG: u16 = 13;
    pub const K2000_DNG: u16 = 14;
    pub const Q_DNG: u16 = 15;
    pub const K200D_DNG: u16 = 16;
    pub const KM_PEF: u16 = 17;
    pub const KX_DNG: u16 = 18;
    pub const KR_DNG: u16 = 19;
    pub const K01_DNG: u16 = 20;
    pub const K30_DNG: u16 = 21;
    pub const K5_IIS_DNG: u16 = 22;
    pub const MX1_DNG: u16 = 23;
    pub const Q10_DNG: u16 = 24;
    pub const Q7_DNG: u16 = 25;
    pub const K3_DNG: u16 = 26;
    pub const K50_DNG: u16 = 27;
    pub const K500_DNG: u16 = 28;
    pub const K200D_PEF: u16 = 29;
    pub const IST_DS_PEF: u16 = 30;
    pub const K5_IIS_PEF: u16 = 31;
    pub const K3_II_DNG: u16 = 32;
    pub const K1_PEF: u16 = 33;
    pub const K1_DNG: u16 = 34;
    pub const K70_PEF: u16 = 35;
    pub const K70_DNG: u16 = 36;
    pub const KS1_PEF: u16 = 37;
    pub const KS1_DNG: u16 = 38;
    pub const KS2_PEF: u16 = 39;
    pub const KS2_DNG: u16 = 40;
    pub const QS1_DNG: u16 = 41;
    pub const _QS1_PEF: u16 = 42;
    pub const KP_PEF: u16 = 43;
    pub const KP_DNG: u16 = 44;
    pub const K1_MKII_PEF: u16 = 45;
    pub const K1_MKII_DNG: u16 = 46;
    pub const K7_DNG: u16 = 47;
    pub const IST_DL2_PEF: u16 = 48;
    pub const K5_II_PEF: u16 = 49;
    pub const K5_II_DNG: u16 = 50;
    pub const K3_PEF: u16 = 51;
    pub const K3_II_PEF: u16 = 52;
    pub const K110D_PEF: u16 = 53;
    pub const K3_MKIII_PEF: u16 = 54;
    pub const K3_MKIII_DNG: u16 = 55;
    pub const K2000_PEF: u16 = 56;
    pub const K5_DNG: u16 = 57;
    pub const K20D_DNG: u16 = 58;
    pub const K3_MKIII_MONO_DNG: u16 = 59;
}

/// Epson type IDs
pub mod epson {
    pub const UNKNOWN: u16 = 0;
    pub const RD1: u16 = 1;
    pub const RD1S: u16 = 2;
    pub const RD1X: u16 = 3;
}

/// Minolta type IDs
pub mod minolta {
    pub const UNKNOWN: u16 = 0;
    pub const A1: u16 = 1;
    pub const A2: u16 = 2;
    pub const DIMAGE5: u16 = 3;
    pub const DIMAGE7: u16 = 4;
    pub const DIMAGE7I: u16 = 5;
    pub const DIMAGE7HI: u16 = 6;
    pub const MAXXUM_5D: u16 = 7;
    pub const MAXXUM_7D: u16 = 8;
    pub const A200: u16 = 9;
}

/// Olympus type IDs
pub mod olympus {
    pub const UNKNOWN: u16 = 0;
    pub const E1: u16 = 1;
    pub const E10: u16 = 2;
    pub const E3: u16 = 3;
    pub const E300: u16 = 4;
    pub const E330: u16 = 5;
    pub const E400: u16 = 6;
    pub const E410: u16 = 7;
    pub const E500: u16 = 8;
    pub const E510: u16 = 9;
    pub const SP350: u16 = 10;
    pub const SP510UZ: u16 = 11;
    pub const SP550UZ: u16 = 12;
    pub const SP500UZ: u16 = 13;
    pub const EP1: u16 = 14;
    pub const E620: u16 = 15;
    pub const EPL1: u16 = 16;
    pub const EP2: u16 = 17;
    pub const XZ1: u16 = 18;
    pub const E5: u16 = 19;
    pub const EPL2: u16 = 20;
    pub const EP3: u16 = 21;
    pub const EPL3: u16 = 22;
    pub const EPM1: u16 = 23;
    pub const EM5: u16 = 24;
    pub const XZ2: u16 = 25;
    pub const EPM2: u16 = 26;
    pub const EPL5: u16 = 27;
    pub const EM1: u16 = 28;
    pub const STYLUS1: u16 = 29;
    pub const EPL6: u16 = 30;
    pub const EPL7: u16 = 31;
    pub const EM5II: u16 = 32;
    pub const EM1II: u16 = 33;
    pub const PEN_F: u16 = 34;
    pub const EM10: u16 = 35;
    pub const EM10II: u16 = 36;
    pub const EPL8: u16 = 37;
    pub const SH2: u16 = 38;
    pub const XZ10: u16 = 39;
    pub const TG4: u16 = 40;
    pub const EPL9: u16 = 41;
    pub const STYLUS1_1S: u16 = 42;
    pub const EM10III: u16 = 43;
    pub const TG5: u16 = 44;
    pub const EM1X: u16 = 45;
    pub const TG6: u16 = 46;
    pub const EM5III: u16 = 47;
    pub const SP565UZ: u16 = 48;
    pub const EPL10: u16 = 49;
    pub const EM1III: u16 = 50;
    pub const EM10IV: u16 = 51;
    pub const EM10IIIS: u16 = 52;
    pub const OM1: u16 = 53;
    pub const E30: u16 = 54;
    pub const EP5: u16 = 55;
    pub const E420: u16 = 56;
    pub const E450: u16 = 57;
    pub const E520: u16 = 58;
    pub const E600: u16 = 59;
    pub const C5060WZ: u16 = 60;
    pub const SP570UZ: u16 = 61;
    pub const EP7: u16 = 62;
    pub const OM5: u16 = 63;
    pub const TG7: u16 = 64;
    pub const OM1II: u16 = 65;
}

/// Samsung type IDs
#[allow(unused)]
pub mod samsung {
    pub const UNKNOWN: u16 = 0;
    pub const GX10: u16 = 1;
    pub const PRO815: u16 = 2;
    pub const GX20: u16 = 3;
}

/// Ricoh type IDs
///
/// Following the merger with Pentax newer cameras may be Pentax.
///
pub mod ricoh {
    pub const GR2: u16 = 1;
    pub const GXR: u16 = 2;
    pub const GXR_A16: u16 = 3;
    /// 2013 Ricoh GR-D
    pub const GR: u16 = 4;
    pub const GX200: u16 = 5;
    pub const PENTAX_645Z_PEF: u16 = 6;
    pub const PENTAX_645Z_DNG: u16 = 7;
    pub const GRII: u16 = 8;
    pub const GRIII: u16 = 9;
    pub const GRIIIX: u16 = 10;
}

/// Sony type IDs
pub mod sony {
    pub const UNKNOWN: u16 = 0;
    pub const A100: u16 = 1;
    pub const A200: u16 = 2;
    pub const A700: u16 = 3;
    pub const A550: u16 = 4;
    pub const A380: u16 = 5;
    pub const A390: u16 = A380;
    pub const SLTA55: u16 = 7;
    pub const SLTA77: u16 = 8;
    pub const NEX3: u16 = 9;
    pub const NEX3N: u16 = 10;
    pub const NEX5: u16 = 11;
    pub const NEX5N: u16 = 12;
    pub const NEX5R: u16 = 13;
    pub const NEX5T: u16 = 14;
    pub const NEX6: u16 = 15;
    pub const NEX7: u16 = 16;
    pub const NEXC3: u16 = 17;
    pub const NEXF3: u16 = 18;
    pub const SLTA65: u16 = 19;
    pub const A330: u16 = 21;
    pub const A450: u16 = 22;
    pub const A580: u16 = 23;
    pub const A850: u16 = 24;
    pub const A900: u16 = 25;
    pub const SLTA35: u16 = 26;
    pub const SLTA33: u16 = 27;
    pub const A560: u16 = 28;
    pub const SLTA99: u16 = 29;
    pub const RX100: u16 = 30;
    pub const RX100M2: u16 = 31;
    pub const RX100M3: u16 = 32;
    pub const RX100M4: u16 = 33;
    pub const RX100M5: u16 = 34;
    pub const RX100M6: u16 = 35;
    pub const RX1: u16 = 36;
    pub const RX1R: u16 = 37;
    pub const RX10: u16 = 38;
    pub const RX10M2: u16 = 39;
    pub const RX10M3: u16 = 40;
    pub const RX1RM2: u16 = 41;
    pub const RX10M4: u16 = 42;
    pub const RX0: u16 = 43;
    pub const SLTA57: u16 = 44;
    pub const ILCE7: u16 = 45;
    pub const ILCE7M2: u16 = 46;
    pub const ILCE7M3: u16 = 47;
    pub const ILCE7R: u16 = 48;
    pub const ILCE7RM2: u16 = 49;
    pub const ILCE7RM3: u16 = 50;
    pub const ILCE7S: u16 = 51;
    pub const ILCE7SM2: u16 = 52;
    pub const ILCE9: u16 = 53;
    pub const ILCE3000: u16 = 54;
    pub const ILCE3500: u16 = ILCE3000;
    pub const SLTA58: u16 = 55;
    pub const ILCE6000: u16 = 56;
    pub const ILCA99M2: u16 = 57;
    pub const ILCE6300: u16 = 58;
    pub const ILCE6500: u16 = 59;
    pub const ILCE5100: u16 = 60;
    pub const A230: u16 = 61;
    pub const A500: u16 = 62;
    pub const SLTA37: u16 = 63;
    pub const ILCA77M2: u16 = 64;
    pub const ILCA68: u16 = 65;
    pub const ILCE5000: u16 = 66;
    pub const A290: u16 = 67;
    pub const RX100M5A: u16 = 68;
    pub const HX99: u16 = 69;
    pub const ILCE6400: u16 = 70;
    pub const RX0M2: u16 = 71;
    pub const ILCE7RM4: u16 = 72;
    pub const RX100M7: u16 = 73;
    pub const ILCE6100: u16 = 74;
    pub const ILCE6600: u16 = 75;
    pub const ILCE9M2: u16 = 76;
    pub const ZV1: u16 = 77;
    // SR2 file
    pub const R1: u16 = 78;
    // ARW
    pub const ILCE7SM3: u16 = 79;
    pub const ILCE7C: u16 = 80;
    pub const ZVE10: u16 = 81;
    pub const ILCE1: u16 = 82;
    pub const ILCE7M4: u16 = 83;
    pub const ILCEQX1: u16 = 84;
    pub const A350: u16 = 85;
    pub const A300: u16 = 86;
    pub const ILCE7RM3A: u16 = 87;
    pub const ILCE7RM4A: u16 = 88;
    pub const HX95: u16 = 89;
    pub const ILCE7RM5: u16 = 90;
    pub const ZVE1: u16 = 91;
    pub const ILME_FX30: u16 = 92;
    pub const ILCE6700: u16 = 93;
    pub const ZV1M2: u16 = 94;
    pub const ILME_FX3: u16 = 95;
    pub const ILCE9M3: u16 = 96;
}

/// Panasonic type IDs
pub mod panasonic {
    pub const UNKNOWN: u16 = 0;
    pub const GF1: u16 = 1;
    pub const GF2: u16 = 2;
    pub const FZ30: u16 = 3;
    pub const G10: u16 = 4;
    pub const GH1: u16 = 5;
    pub const GH2: u16 = 6;
    pub const LX2: u16 = 7;
    pub const LX3: u16 = 8;
    pub const LX5: u16 = 9;
    pub const FZ8: u16 = 10;
    pub const FZ18: u16 = 11;
    pub const FZ50: u16 = 12;
    pub const L1: u16 = 13;
    pub const G1: u16 = 14;
    pub const G2: u16 = 15;
    pub const L10: u16 = 16;
    pub const FZ28: u16 = 17;
    pub const GF3: u16 = 18;
    pub const FZ100: u16 = 19;
    pub const GX1: u16 = 20;
    pub const G3: u16 = 21;
    pub const G5: u16 = 22;
    pub const GF5: u16 = 23;
    pub const LX7: u16 = 24;
    pub const GH3: u16 = 25;
    pub const FZ200: u16 = 26;
    pub const GF6: u16 = 27;
    pub const GX7: u16 = 28;
    pub const GM1: u16 = 29;
    pub const GM1S: u16 = GM1;
    pub const GH4: u16 = 30;
    pub const LX100: u16 = 31;
    pub const GM5: u16 = 32;
    pub const G80: u16 = 33;
    #[allow(unused)]
    pub const G85: u16 = G80;
    pub const LX10: u16 = 34;
    pub const LX15: u16 = LX10;
    pub const FZ2500: u16 = 35;
    pub const FZ2000: u16 = FZ2500;
    pub const GX8: u16 = 36;
    pub const ZS100: u16 = 37;
    pub const TX1: u16 = ZS100;
    pub const TZ100: u16 = ZS100;
    pub const TZ101: u16 = ZS100;
    pub const TZ110: u16 = ZS100;
    pub const GX80: u16 = 38;
    pub const GX85: u16 = GX80;
    pub const GH5: u16 = 39;
    pub const GX850: u16 = 40;
    pub const FZ80: u16 = 41;
    pub const FZ82: u16 = FZ80;
    pub const FZ330: u16 = 42;
    pub const TZ70: u16 = 43;
    pub const ZS60: u16 = 44;
    pub const TZ80: u16 = ZS60;
    pub const TZ81: u16 = ZS60;

    pub const GF7: u16 = 46;
    pub const CM1: u16 = 47;
    pub const GX9: u16 = 48;
    pub const GX7MK3: u16 = GX9;
    pub const GX800: u16 = 49;

    pub const G9: u16 = 52;
    pub const DC_FZ45: u16 = 53; // Not the DMC FZ45
    pub const GH5S: u16 = 54;
    pub const LX1: u16 = 55;
    pub const FZ150: u16 = 56;
    pub const FZ35: u16 = 57;
    pub const FZ38: u16 = FZ35;
    pub const ZS200: u16 = 58;
    #[allow(unused)]
    pub const TX2: u16 = ZS200;
    pub const TZ202: u16 = ZS200;
    pub const ZS200D: u16 = ZS200;
    pub const GX7MK2: u16 = 59;
    pub const LX100M2: u16 = 60;
    pub const DMC_FZ40: u16 = 61;
    pub const DMC_FZ45: u16 = DMC_FZ40; // Not the DC FZ45
    pub const DC_S1: u16 = 62;
    pub const DC_S1R: u16 = 63;
    pub const DC_G95: u16 = 64;
    pub const DMC_FZ1000: u16 = 65;
    pub const DC_FZ1000M2: u16 = 66;
    pub const DC_ZS80: u16 = 67;
    pub const DC_TZ95: u16 = DC_ZS80;
    pub const DC_TZ96: u16 = DC_ZS80;
    pub const GF10: u16 = 68;
    pub const GX880: u16 = GF10;
    pub const DC_G99: u16 = 69;
    pub const DC_G90: u16 = DC_G99;
    pub const DC_G91: u16 = DC_G99;
    pub const DC_G95D: u16 = DC_G99;
    pub const DC_S1H: u16 = 70;
    pub const DC_G100: u16 = 71;
    pub const DC_G110: u16 = DC_G100;
    pub const DC_S5: u16 = 72;
    pub const GH5M2: u16 = 73;
    pub const GH6: u16 = 74;
    pub const G7: u16 = 75;
    pub const G70: u16 = G7;
    pub const ZS40: u16 = 76;
    pub const TZ60: u16 = ZS40;
    pub const TZ61: u16 = ZS40;
    pub const G6: u16 = 77;
    pub const G8: u16 = 78;
    pub const G81: u16 = G8;
    pub const LF1: u16 = 79;
    pub const FZ300: u16 = 80;
    pub const FZ70: u16 = 81;
    pub const FZ72: u16 = FZ70;
    pub const ZS50: u16 = 82;
    pub const TZ71: u16 = ZS50;
    pub const ZS70: u16 = 83;
    pub const TZ90: u16 = ZS70;
    pub const DC_S5M2: u16 = 84;
    pub const GF8: u16 = 85;
    pub const DC_S5M2X: u16 = 86;
}

/// Fujifilm type IDs
pub mod fujifilm {
    pub const UNKNOWN: u16 = 0;
    pub const F700: u16 = 1;
    pub const E900: u16 = 2;
    pub const S2PRO: u16 = 3;
    pub const S3PRO: u16 = 4;
    pub const S5PRO: u16 = 5;
    pub const F810: u16 = 6;
    pub const S5000: u16 = 7;
    pub const S5600: u16 = 8;
    pub const S9500: u16 = 9;
    pub const S6500FD: u16 = 10;
    pub const HS10: u16 = 11;
    pub const HS30EXR: u16 = 12;
    pub const HS33EXR: u16 = HS30EXR;
    pub const S200EXR: u16 = 13;
    pub const X100: u16 = 14;
    pub const X100S: u16 = 15;
    pub const X100T: u16 = 16;
    pub const X100F: u16 = 17;
    pub const X10: u16 = 18;
    pub const X20: u16 = 19;
    pub const X30: u16 = 20;
    pub const X70: u16 = 21;
    pub const XPRO1: u16 = 22;
    pub const XPRO2: u16 = 23;
    pub const XS1: u16 = 24;
    pub const XE1: u16 = 25;
    pub const XE2: u16 = 26;
    pub const XE2S: u16 = 27;
    pub const XE3: u16 = 28;
    pub const XF1: u16 = 29;
    pub const XM1: u16 = 30;
    pub const XT1: u16 = 31;
    pub const XT10: u16 = 32;
    pub const XT100: u16 = 33;
    pub const XT2: u16 = 34;
    pub const XT20: u16 = 35;
    pub const XT3: u16 = 36;
    pub const XA1: u16 = 37;
    pub const XA2: u16 = 38;
    pub const XA3: u16 = 39;
    pub const XA5: u16 = 40;
    pub const XQ1: u16 = 41;
    pub const XQ2: u16 = 42;
    pub const XH1: u16 = 43;
    pub const GFX50S: u16 = 44;
    pub const GFX50R: u16 = 45;
    pub const XF10: u16 = 46;
    pub const XT30: u16 = 47;
    pub const GFX100: u16 = 48;
    pub const XA7: u16 = 49;
    pub const XPRO3: u16 = 50;
    pub const XT200: u16 = 51;
    pub const X100V: u16 = 52;
    pub const XT4: u16 = 53;
    pub const F550EXR: u16 = 54;
    pub const S100FS: u16 = 55;
    pub const XS10: u16 = 56;
    pub const XT30_II: u16 = 57;
    pub const GFX50S_II: u16 = 58;
    pub const GFX100S: u16 = 59;
    pub const XE4: u16 = 60;
    pub const XA10: u16 = 61;
    pub const XH2S: u16 = 62;
    pub const XH2: u16 = 63;
    pub const XT5: u16 = 64;
    pub const HS50EXR: u16 = 65;
    pub const S6000FD: u16 = 66;
    pub const SL1000: u16 = 67;
    pub const XS20: u16 = 68;
}

/// Black Magic
pub mod blackmagic {
    pub const POCKET_CINEMA: u16 = 1;
}

/// Xiaoyi
pub mod xiaoyi {
    pub const M1: u16 = 1;
    pub const YDXJ_2: u16 = 2;
    pub const YIAC_3: u16 = 3;
}

/// Apple (iPhones)
pub mod apple {
    pub const IPHONE_6SPLUS: u16 = 1;
    pub const IPHONE_7PLUS: u16 = 2;
    pub const IPHONE_SE: u16 = 3;
    pub const IPHONE_8: u16 = 4;
    pub const IPHONE_XS: u16 = 5;
    pub const IPHONE_12_PRO: u16 = 6;
    pub const IPHONE_13_PRO: u16 = 7;
    pub const IPHONE_14: u16 = 8;
    pub const IPHONE_15_PRO: u16 = 9;
}

/// Sigma
pub mod sigma {
    pub const FP: u16 = 1;
    pub const FP_L: u16 = 2;
}

/// GoPro
pub mod gopro {
    pub const HERO5_BLACK: u16 = 1;
    pub const HERO6_BLACK: u16 = 2;
    pub const HERO7_BLACK: u16 = 3;
    pub const HERO8_BLACK: u16 = 4;
    pub const HERO9_BLACK: u16 = 5;
    pub const HERO10_BLACK: u16 = 6;
    pub const HERO11_BLACK: u16 = 7;
    pub const HERO12_BLACK: u16 = 8;
    pub const FUSION: u16 = 9;
}

/// Hasselblad
pub mod hasselblad {
    pub const LUNAR: u16 = 1;
    pub const L1D_20C: u16 = 2;
    pub const L2D_20C: u16 = 3;
}

/// Zeiss
pub mod zeiss {
    pub const ZX1: u16 = 1;
}

/// DJI
pub mod dji {
    pub const FC350: u16 = 1;
    pub const FC7303: u16 = 2;
    pub const OSMO_ACTION: u16 = 3;
    pub const FC220: u16 = 4;
    pub const FC6310: u16 = 5;
    pub const FC3582: u16 = 6;
}

pub mod nokia {
    pub const LUMIA_1020: u16 = 1;
}
