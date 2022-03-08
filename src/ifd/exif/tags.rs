/*
 * libopenraw - ifd/exif/tags.rs
 *
 * Copyright (C) 2022 Hubert Figuière
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

pub const EXIF_TAG_INTEROPERABILITY_INDEX: u16 = 0x0001;
pub const EXIF_TAG_INTEROPERABILITY_VERSION: u16 = 0x0002;
pub const EXIF_TAG_NEW_SUBFILE_TYPE: u16 = 0x00fe;
pub const EXIF_TAG_IMAGE_WIDTH: u16 = 0x0100;
pub const EXIF_TAG_IMAGE_LENGTH: u16 = 0x0101;
pub const EXIF_TAG_BITS_PER_SAMPLE: u16 = 0x0102;
pub const EXIF_TAG_COMPRESSION: u16 = 0x0103;
pub const EXIF_TAG_PHOTOMETRIC_INTERPRETATION: u16 = 0x0106;
pub const EXIF_TAG_FILL_ORDER: u16 = 0x010a;
pub const EXIF_TAG_DOCUMENT_NAME: u16 = 0x010d;
pub const EXIF_TAG_IMAGE_DESCRIPTION: u16 = 0x010e;
pub const EXIF_TAG_MAKE: u16 = 0x010f;
pub const EXIF_TAG_MODEL: u16 = 0x0110;
pub const EXIF_TAG_STRIP_OFFSETS: u16 = 0x0111;
pub const EXIF_TAG_ORIENTATION: u16 = 0x0112;
pub const EXIF_TAG_SAMPLES_PER_PIXEL: u16 = 0x0115;
pub const EXIF_TAG_ROWS_PER_STRIP: u16 = 0x0116;
pub const EXIF_TAG_STRIP_BYTE_COUNTS: u16 = 0x0117;
pub const EXIF_TAG_X_RESOLUTION: u16 = 0x011a;
pub const EXIF_TAG_Y_RESOLUTION: u16 = 0x011b;
pub const EXIF_TAG_PLANAR_CONFIGURATION: u16 = 0x011c;
pub const EXIF_TAG_RESOLUTION_UNIT: u16 = 0x0128;
pub const EXIF_TAG_TRANSFER_FUNCTION: u16 = 0x012d;
pub const EXIF_TAG_SOFTWARE: u16 = 0x0131;
pub const EXIF_TAG_DATE_TIME: u16 = 0x0132;
pub const EXIF_TAG_ARTIST: u16 = 0x013b;
pub const EXIF_TAG_WHITE_POINT: u16 = 0x013e;
pub const EXIF_TAG_PRIMARY_CHROMATICITIES: u16 = 0x013f;
pub const TIFF_TAG_TILE_WIDTH: u16 = 0x0142;
pub const TIFF_TAG_TILE_LENGTH: u16 = 0x0143;
pub const TIFF_TAG_TILE_OFFSETS: u16 = 0x0144;
pub const TIFF_TAG_TILE_BYTECOUNTS: u16 = 0x0145;
pub const EXIF_TAG_TRANSFER_RANGE: u16 = 0x0156;
pub const EXIF_TAG_SUB_IFDS: u16 = 0x014a;
pub const EXIF_TAG_JPEG_PROC: u16 = 0x0200;
pub const EXIF_TAG_JPEG_INTERCHANGE_FORMAT: u16 = 0x0201;
pub const EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH: u16 = 0x0202;
pub const EXIF_TAG_YCBCR_COEFFICIENTS: u16 = 0x0211;
pub const EXIF_TAG_YCBCR_SUB_SAMPLING: u16 = 0x0212;
pub const EXIF_TAG_YCBCR_POSITIONING: u16 = 0x0213;
pub const EXIF_TAG_REFERENCE_BLACK_WHITE: u16 = 0x0214;
pub const EXIF_TAG_XML_PACKET: u16 = 0x02bc;
pub const EXIF_TAG_RELATED_IMAGE_FILE_FORMAT: u16 = 0x1000;
pub const EXIF_TAG_RELATED_IMAGE_WIDTH: u16 = 0x1001;
pub const EXIF_TAG_RELATED_IMAGE_LENGTH: u16 = 0x1002;
pub const EXIF_TAG_CFA_REPEAT_PATTERN_DIM: u16 = 0x828d;
pub const EXIF_TAG_CFA_PATTERN: u16 = 0x828e;
pub const EXIF_TAG_BATTERY_LEVEL: u16 = 0x828f;
pub const EXIF_TAG_COPYRIGHT: u16 = 0x8298;
pub const EXIF_TAG_EXPOSURE_TIME: u16 = 0x829a;
pub const EXIF_TAG_FNUMBER: u16 = 0x829d;
pub const EXIF_TAG_IPTC_NAA: u16 = 0x83bb;
pub const EXIF_TAG_IMAGE_RESOURCES: u16 = 0x8649;
pub const EXIF_TAG_EXIF_IFD_POINTER: u16 = 0x8769;
pub const EXIF_TAG_INTER_COLOR_PROFILE: u16 = 0x8773;
pub const EXIF_TAG_EXPOSURE_PROGRAM: u16 = 0x8822;
pub const EXIF_TAG_SPECTRAL_SENSITIVITY: u16 = 0x8824;
pub const EXIF_TAG_GPS_INFO_IFD_POINTER: u16 = 0x8825;
pub const EXIF_TAG_ISO_SPEED_RATINGS: u16 = 0x8827;
pub const EXIF_TAG_OECF: u16 = 0x8828;
pub const EXIF_TAG_EXIF_VERSION: u16 = 0x9000;
pub const EXIF_TAG_DATE_TIME_ORIGINAL: u16 = 0x9003;
pub const EXIF_TAG_DATE_TIME_DIGITIZED: u16 = 0x9004;
pub const EXIF_TAG_COMPONENTS_CONFIGURATION: u16 = 0x9101;
pub const EXIF_TAG_COMPRESSED_BITS_PER_PIXEL: u16 = 0x9102;
pub const EXIF_TAG_SHUTTER_SPEED_VALUE: u16 = 0x9201;
pub const EXIF_TAG_APERTURE_VALUE: u16 = 0x9202;
pub const EXIF_TAG_BRIGHTNESS_VALUE: u16 = 0x9203;
pub const EXIF_TAG_EXPOSURE_BIAS_VALUE: u16 = 0x9204;
pub const EXIF_TAG_MAX_APERTURE_VALUE: u16 = 0x9205;
pub const EXIF_TAG_SUBJECT_DISTANCE: u16 = 0x9206;
pub const EXIF_TAG_METERING_MODE: u16 = 0x9207;
pub const EXIF_TAG_LIGHT_SOURCE: u16 = 0x9208;
pub const EXIF_TAG_FLASH: u16 = 0x9209;
pub const EXIF_TAG_FOCAL_LENGTH: u16 = 0x920a;
pub const EXIF_TAG_SUBJECT_AREA: u16 = 0x9214;
pub const EXIF_TAG_TIFF_EP_STANDARD_ID: u16 = 0x9216;
pub const EXIF_TAG_MAKER_NOTE: u16 = 0x927c;
pub const EXIF_TAG_USER_COMMENT: u16 = 0x9286;
pub const EXIF_TAG_SUB_SEC_TIME: u16 = 0x9290;
pub const EXIF_TAG_SUB_SEC_TIME_ORIGINAL: u16 = 0x9291;
pub const EXIF_TAG_SUB_SEC_TIME_DIGITIZED: u16 = 0x9292;
pub const EXIF_TAG_FLASH_PIX_VERSION: u16 = 0xa000;
pub const EXIF_TAG_COLOR_SPACE: u16 = 0xa001;
pub const EXIF_TAG_PIXEL_X_DIMENSION: u16 = 0xa002;
pub const EXIF_TAG_PIXEL_Y_DIMENSION: u16 = 0xa003;
pub const EXIF_TAG_RELATED_SOUND_FILE: u16 = 0xa004;
pub const EXIF_TAG_INTEROPERABILITY_IFD_POINTER: u16 = 0xa005;
pub const EXIF_TAG_FLASH_ENERGY: u16 = 0xa20b;
pub const EXIF_TAG_SPATIAL_FREQUENCY_RESPONSE: u16 = 0xa20c;
pub const EXIF_TAG_FOCAL_PLANE_X_RESOLUTION: u16 = 0xa20e;
pub const EXIF_TAG_FOCAL_PLANE_Y_RESOLUTION: u16 = 0xa20f;
pub const EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT: u16 = 0xa210;
pub const EXIF_TAG_SUBJECT_LOCATION: u16 = 0xa214;
pub const EXIF_TAG_EXPOSURE_INDEX: u16 = 0xa215;
pub const EXIF_TAG_SENSING_METHOD: u16 = 0xa217;
pub const EXIF_TAG_FILE_SOURCE: u16 = 0xa300;
pub const EXIF_TAG_SCENE_TYPE: u16 = 0xa301;
pub const EXIF_TAG_NEW_CFA_PATTERN: u16 = 0xa302;
pub const EXIF_TAG_CUSTOM_RENDERED: u16 = 0xa401;
pub const EXIF_TAG_EXPOSURE_MODE: u16 = 0xa402;
pub const EXIF_TAG_WHITE_BALANCE: u16 = 0xa403;
pub const EXIF_TAG_DIGITAL_ZOOM_RATIO: u16 = 0xa404;
pub const EXIF_TAG_FOCAL_LENGTH_IN_35MM_FILM: u16 = 0xa405;
pub const EXIF_TAG_SCENE_CAPTURE_TYPE: u16 = 0xa406;
pub const EXIF_TAG_GAIN_CONTROL: u16 = 0xa407;
pub const EXIF_TAG_CONTRAST: u16 = 0xa408;
pub const EXIF_TAG_SATURATION: u16 = 0xa409;
pub const EXIF_TAG_SHARPNESS: u16 = 0xa40a;
pub const EXIF_TAG_DEVICE_SETTING_DESCRIPTION: u16 = 0xa40b;
pub const EXIF_TAG_SUBJECT_DISTANCE_RANGE: u16 = 0xa40c;
pub const EXIF_TAG_IMAGE_UNIQUE_ID: u16 = 0xa420;
pub const EXIF_TAG_CAMERA_OWNER_NAME: u16 = 0xa430;
pub const EXIF_TAG_BODY_SERIAL_NUMBER: u16 = 0xa431;
pub const EXIF_TAG_BODY_LENS_MAKE: u16 = 0xa433;
pub const EXIF_TAG_BODY_LENS_MODEL: u16 = 0xa434;
pub const EXIF_TAG_GAMMA: u16 = 0xa500;
pub const EXIF_TAG_UNKNOWN_C4A5: u16 = 0xc4a5;
/* DNG tags */
pub const TIFF_TAG_DNG_VERSION: u16 = 0xc612;
pub const DNG_TAG_UNIQUE_CAMERA_MODEL: u16 = 0xc614;
pub const DNG_TAG_DEFAULT_CROP_ORIGIN: u16 = 0xc61f;
pub const DNG_TAG_DEFAULT_CROP_SIZE: u16 = 0xc620;
pub const DNG_TAG_COLORMATRIX1: u16 = 0xc621;
pub const DNG_TAG_COLORMATRIX2: u16 = 0xc622;
pub const DNG_TAG_CAMERA_CALIBRATION1: u16 = 0xc623;
pub const DNG_TAG_CAMERA_CALIBRATION2: u16 = 0xc624;
pub const DNG_TAG_REDUCTION_MATRIX1: u16 = 0xc625;
pub const DNG_TAG_REDUCTION_MATRIX2: u16 = 0xc626;
pub const DNG_TAG_ANALOG_BALANCE: u16 = 0xc627;
pub const DNG_TAG_AS_SHOT_NEUTRAL: u16 = 0xc628;
pub const DNG_TAG_AS_SHOT_WHITE_XY: u16 = 0xc629;
pub const DNG_TAG_CALIBRATION_ILLUMINANT1: u16 = 0xc65a;
pub const DNG_TAG_CALIBRATION_ILLUMINANT2: u16 = 0xc65b;
pub const DNG_TAG_ORIGINAL_RAW_FILE_NAME: u16 = 0xc68b;
pub const DNG_TAG_ACTIVE_AREA: u16 = 0xc68d;

/* ERF tags */
pub const ERF_TAG_PREVIEW_IMAGE: u16 = 0x280;

/* ERF MakerNote */
pub const MNOTE_EPSON_SENSORAREA: u16 = 0x400;

/* ORF tags */
pub const ORF_TAG_THUMBNAIL_IMAGE: u16 = 0x100;
pub const ORF_TAG_CAMERA_SETTINGS: u16 = 0x2020;
/* Camera Settings */
pub const ORF_TAG_CS_PREVIEW_IMAGE_VALID: u16 = 0x100;
pub const ORF_TAG_CS_PREVIEW_IMAGE_START: u16 = 0x101;
pub const ORF_TAG_CS_PREVIEW_IMAGE_LENGTH: u16 = 0x102;

/* CR2 tags */
pub const CR2_TAG_C5D9: u16 = 0xc5d9;
/// Exif tag for CR2 RAW "slices"
pub const CR2_TAG_SLICE: u16 = 0xc640;
pub const CR2_TAG_SRAW_TYPE: u16 = 0xc6c5;
pub const CR2_TAG_C6D6: u16 = 0xc6d6;

/* RW2 tags */
pub const RW2_TAG_SENSOR_WIDTH: u16 = 0x0002;
pub const RW2_TAG_SENSOR_HEIGHT: u16 = 0x0003;
pub const RW2_TAG_SENSOR_TOPBORDER: u16 = 0x0004;
pub const RW2_TAG_SENSOR_LEFTBORDER: u16 = 0x0005;
pub const RW2_TAG_SENSOR_BOTTOMBORDER: u16 = 0x0006;
pub const RW2_TAG_SENSOR_RIGHTBORDER: u16 = 0x0007;
pub const RW2_TAG_IMAGE_CFAPATTERN: u16 = 0x0009;
pub const RW2_TAG_IMAGE_BITSPERSAMPLE: u16 = 0x000a;
pub const RW2_TAG_IMAGE_COMPRESSION: u16 = 0x000b;
pub const RW2_TAG_IMAGE_RAWFORMAT: u16 = 0x002d;
pub const RW2_TAG_JPEG_FROM_RAW: u16 = 0x002e;
pub const RW2_TAG_STRIP_OFFSETS: u16 = 0x0118;

/* Pentax MakerNote tags */
pub const MNOTE_PENTAX_PREVIEW_IMAGE_SIZE: u16 = 0x02;
pub const MNOTE_PENTAX_PREVIEW_IMAGE_LENGTH: u16 = 0x03;
pub const MNOTE_PENTAX_PREVIEW_IMAGE_START: u16 = 0x04;
pub const MNOTE_PENTAX_MODEL_ID: u16 = 0x05;
pub const MNOTE_PENTAX_IMAGEAREAOFFSET: u16 = 0x38;
pub const MNOTE_PENTAX_RAWIMAGESIZE: u16 = 0x39;
pub const MNOTE_PENTAX_WHITELEVEL: u16 = 0x7e;

/* Canon MakerNote tags */
/// The model ID for Canon cameras.
pub const MNOTE_CANON_MODEL_ID: u16 = 0x0010;
pub const MNOTE_CANON_RAW_DATA_OFFSET: u16 = 0x0081;
pub const MNOTE_CANON_RAW_DATA_LENGTH: u16 = 0x0082;
pub const MNOTE_CANON_SENSORINFO: u16 = 0x00e0;

/* Leica MakerNote tags */
pub const MNOTE_LEICA_PREVIEW_IMAGE: u16 = 0x300;

/* Nikon MakerNote tags */
pub const MNOTE_NIKON_QUALITY: u16 = 0x04;
pub const MNOTE_NIKON_PREVIEW_IFD: u16 = 0x11;
pub const MNOTE_NIKON_NEFDECODETABLE2: u16 = 0x96;

/* Nikon MakerNote Preview IFD tags */
pub const MNOTE_NIKON_PREVIEWIFD_START: u16 = 0x201;
pub const MNOTE_NIKON_PREVIEWIFD_LENGTH: u16 = 0x202;

/* Sony MakerNode */
pub const MNOTE_SONY_MODEL_ID: u16 = 0xb001;
