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

use std::collections::HashMap;

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
pub const CR2_TAG_C5D8: u16 = 0xc5d8;
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

lazy_static::lazy_static! {
    /// Exif tag names
    pub static ref TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x1, "InteropIndex"),
        (0x2, "InteropVersion"),
        (0xb, "ProcessingSoftware"),
        (0xfe, "SubfileType"),
        (0xff, "OldSubfileType"),
        (0x100, "ImageWidth"),
        (0x101, "ImageHeight"),
        (0x102, "BitsPerSample"),
        (0x103, "Compression"),
        (0x106, "PhotometricInterpretation"),
        (0x107, "Thresholding"),
        (0x108, "CellWidth"),
        (0x109, "CellLength"),
        (0x10a, "FillOrder"),
        (0x10d, "DocumentName"),
        (0x10e, "ImageDescription"),
        (0x10f, "Make"),
        (0x110, "Model"),
        (0x111, "StripOffsets"),
        (0x112, "Orientation"),
        (0x115, "SamplesPerPixel"),
        (0x116, "RowsPerStrip"),
        (0x117, "StripByteCounts"),
        (0x118, "MinSampleValue"),
        (0x119, "MaxSampleValue"),
        (0x11a, "XResolution"),
        (0x11b, "YResolution"),
        (0x11c, "PlanarConfiguration"),
        (0x11d, "PageName"),
        (0x11e, "XPosition"),
        (0x11f, "YPosition"),
        (0x120, "FreeOffsets"),
        (0x121, "FreeByteCounts"),
        (0x122, "GrayResponseUnit"),
        (0x123, "GrayResponseCurve"),
        (0x124, "T4Options"),
        (0x125, "T6Options"),
        (0x128, "ResolutionUnit"),
        (0x129, "PageNumber"),
        (0x12c, "ColorResponseUnit"),
        (0x12d, "TransferFunction"),
        (0x131, "Software"),
        (0x132, "ModifyDate"),
        (0x13b, "Artist"),
        (0x13c, "HostComputer"),
        (0x13d, "Predictor"),
        (0x13e, "WhitePoint"),
        (0x13f, "PrimaryChromaticities"),
        (0x140, "ColorMap"),
        (0x141, "HalftoneHints"),
        (0x142, "TileWidth"),
        (0x143, "TileLength"),
        (0x144, "TileOffsets"),
        (0x145, "TileByteCounts"),
        (0x146, "BadFaxLines"),
        (0x147, "CleanFaxData"),
        (0x148, "ConsecutiveBadFaxLines"),
        (0x14a, "SubIFD"),
        (0x14c, "InkSet"),
        (0x14d, "InkNames"),
        (0x14e, "NumberofInks"),
        (0x150, "DotRange"),
        (0x151, "TargetPrinter"),
        (0x152, "ExtraSamples"),
        (0x153, "SampleFormat"),
        (0x154, "SMinSampleValue"),
        (0x155, "SMaxSampleValue"),
        (0x156, "TransferRange"),
        (0x157, "ClipPath"),
        (0x158, "XClipPathUnits"),
        (0x159, "YClipPathUnits"),
        (0x15a, "Indexed"),
        (0x15b, "JPEGTables"),
        (0x15f, "OPIProxy"),
        (0x190, "GlobalParametersIFD"),
        (0x191, "ProfileType"),
        (0x192, "FaxProfile"),
        (0x193, "CodingMethods"),
        (0x194, "VersionYear"),
        (0x195, "ModeNumber"),
        (0x1b1, "Decode"),
        (0x1b2, "DefaultImageColor"),
        (0x1b3, "T82Options"),
        (0x1b5, "JPEGTables"),
        (0x200, "JPEGProc"),
        (0x201, "ThumbnailOffset"),
        (0x202, "ThumbnailLength"),
        (0x203, "JPEGRestartInterval"),
        (0x205, "JPEGLosslessPredictors"),
        (0x206, "JPEGPointTransforms"),
        (0x207, "JPEGQTables"),
        (0x208, "JPEGDCTables"),
        (0x209, "JPEGACTables"),
        (0x211, "YCbCrCoefficients"),
        (0x212, "YCbCrSubSampling"),
        (0x213, "YCbCrPositioning"),
        (0x214, "ReferenceBlackWhite"),
        (0x22f, "StripRowCounts"),
        (0x2bc, "ApplicationNotes"),
        (0x3e7, "USPTOMiscellaneous"),
        (0x1000, "RelatedImageFileFormat"),
        (0x1001, "RelatedImageWidth"),
        (0x1002, "RelatedImageHeight"),
        (0x4746, "Rating"),
        (0x4747, "XP_DIP_XML"),
        (0x4748, "StitchInfo"),
        (0x4749, "RatingPercent"),
        (0x7000, "SonyRawFileType"),
        (0x7010, "SonyToneCurve"),
        (0x7031, "VignettingCorrection"),
        (0x7032, "VignettingCorrParams"),
        (0x7034, "ChromaticAberrationCorrection"),
        (0x7035, "ChromaticAberrationCorrParams"),
        (0x7036, "DistortionCorrection"),
        (0x7037, "DistortionCorrParams"),
        (0x74c7, "SonyCropTopLeft"),
        (0x74c8, "SonyCropSize"),
        (0x800d, "ImageID"),
        (0x80a3, "WangTag1"),
        (0x80a4, "WangAnnotation"),
        (0x80a5, "WangTag3"),
        (0x80a6, "WangTag4"),
        (0x80b9, "ImageReferencePoints"),
        (0x80ba, "RegionXformTackPoint"),
        (0x80bb, "WarpQuadrilateral"),
        (0x80bc, "AffineTransformMat"),
        (0x80e3, "Matteing"),
        (0x80e4, "DataType"),
        (0x80e5, "ImageDepth"),
        (0x80e6, "TileDepth"),
        (0x8214, "ImageFullWidth"),
        (0x8215, "ImageFullHeight"),
        (0x8216, "TextureFormat"),
        (0x8217, "WrapModes"),
        (0x8218, "FovCot"),
        (0x8219, "MatrixWorldToScreen"),
        (0x821a, "MatrixWorldToCamera"),
        (0x827d, "Model2"),
        (0x828d, "CFARepeatPatternDim"),
        (0x828e, "CFAPattern2"),
        (0x828f, "BatteryLevel"),
        (0x8290, "KodakIFD"),
        (0x8298, "Copyright"),
        (0x829a, "ExposureTime"),
        (0x829d, "FNumber"),
        (0x82a5, "MDFileTag"),
        (0x82a6, "MDScalePixel"),
        (0x82a7, "MDColorTable"),
        (0x82a8, "MDLabName"),
        (0x82a9, "MDSampleInfo"),
        (0x82aa, "MDPrepDate"),
        (0x82ab, "MDPrepTime"),
        (0x82ac, "MDFileUnits"),
        (0x830e, "PixelScale"),
        (0x8335, "AdventScale"),
        (0x8336, "AdventRevision"),
        (0x835c, "UIC1Tag"),
        (0x835d, "UIC2Tag"),
        (0x835e, "UIC3Tag"),
        (0x835f, "UIC4Tag"),
        (0x83bb, "IPTC-NAA"),
        (0x847e, "IntergraphPacketData"),
        (0x847f, "IntergraphFlagRegisters"),
        (0x8480, "IntergraphMatrix"),
        (0x8481, "INGRReserved"),
        (0x8482, "ModelTiePoint"),
        (0x84e0, "Site"),
        (0x84e1, "ColorSequence"),
        (0x84e2, "IT8Header"),
        (0x84e3, "RasterPadding"),
        (0x84e4, "BitsPerRunLength"),
        (0x84e5, "BitsPerExtendedRunLength"),
        (0x84e6, "ColorTable"),
        (0x84e7, "ImageColorIndicator"),
        (0x84e8, "BackgroundColorIndicator"),
        (0x84e9, "ImageColorValue"),
        (0x84ea, "BackgroundColorValue"),
        (0x84eb, "PixelIntensityRange"),
        (0x84ec, "TransparencyIndicator"),
        (0x84ed, "ColorCharacterization"),
        (0x84ee, "HCUsage"),
        (0x84ef, "TrapIndicator"),
        (0x84f0, "CMYKEquivalent"),
        (0x8546, "SEMInfo"),
        (0x8568, "AFCP_IPTC"),
        (0x85b8, "PixelMagicJBIGOptions"),
        (0x85d7, "JPLCartoIFD"),
        (0x85d8, "ModelTransform"),
        (0x8602, "WB_GRGBLevels"),
        (0x8606, "LeafData"),
        (0x8649, "PhotoshopSettings"),
        (0x8769, "ExifOffset"),
        (0x8773, "ICC_Profile"),
        (0x877f, "TIFF_FXExtensions"),
        (0x8780, "MultiProfiles"),
        (0x8781, "SharedData"),
        (0x8782, "T88Options"),
        (0x87ac, "ImageLayer"),
        (0x87af, "GeoTiffDirectory"),
        (0x87b0, "GeoTiffDoubleParams"),
        (0x87b1, "GeoTiffAsciiParams"),
        (0x87be, "JBIGOptions"),
        (0x8822, "ExposureProgram"),
        (0x8824, "SpectralSensitivity"),
        (0x8825, "GPSInfo"),
        (0x8827, "ISO"),
        (0x8828, "Opto-ElectricConvFactor"),
        (0x8829, "Interlace"),
        (0x882a, "TimeZoneOffset"),
        (0x882b, "SelfTimerMode"),
        (0x8830, "SensitivityType"),
        (0x8831, "StandardOutputSensitivity"),
        (0x8832, "RecommendedExposureIndex"),
        (0x8833, "ISOSpeed"),
        (0x8834, "ISOSpeedLatitudeyyy"),
        (0x8835, "ISOSpeedLatitudezzz"),
        (0x885c, "FaxRecvParams"),
        (0x885d, "FaxSubAddress"),
        (0x885e, "FaxRecvTime"),
        (0x8871, "FedexEDR"),
        (0x888a, "LeafSubIFD"),
        (0x9000, "ExifVersion"),
        (0x9003, "DateTimeOriginal"),
        (0x9004, "CreateDate"),
        (0x9009, "GooglePlusUploadCode"),
        (0x9010, "OffsetTime"),
        (0x9011, "OffsetTimeOriginal"),
        (0x9012, "OffsetTimeDigitized"),
        (0x9101, "ComponentsConfiguration"),
        (0x9102, "CompressedBitsPerPixel"),
        (0x9201, "ShutterSpeedValue"),
        (0x9202, "ApertureValue"),
        (0x9203, "BrightnessValue"),
        (0x9204, "ExposureCompensation"),
        (0x9205, "MaxApertureValue"),
        (0x9206, "SubjectDistance"),
        (0x9207, "MeteringMode"),
        (0x9208, "LightSource"),
        (0x9209, "Flash"),
        (0x920a, "FocalLength"),
        (0x920b, "FlashEnergy"),
        (0x920c, "SpatialFrequencyResponse"),
        (0x920d, "Noise"),
        (0x920e, "FocalPlaneXResolution"),
        (0x920f, "FocalPlaneYResolution"),
        (0x9210, "FocalPlaneResolutionUnit"),
        (0x9211, "ImageNumber"),
        (0x9212, "SecurityClassification"),
        (0x9213, "ImageHistory"),
        (0x9214, "SubjectArea"),
        (0x9215, "ExposureIndex"),
        (0x9216, "TIFF-EPStandardID"),
        (0x9217, "SensingMethod"),
        (0x923a, "CIP3DataFile"),
        (0x923b, "CIP3Sheet"),
        (0x923c, "CIP3Side"),
        (0x923f, "StoNits"),
        (0x927c, "MakerNoteApple"),
        (0x9286, "UserComment"),
        (0x9290, "SubSecTime"),
        (0x9291, "SubSecTimeOriginal"),
        (0x9292, "SubSecTimeDigitized"),
        (0x932f, "MSDocumentText"),
        (0x9330, "MSPropertySetStorage"),
        (0x9331, "MSDocumentTextPosition"),
        (0x935c, "ImageSourceData"),
        (0x9400, "AmbientTemperature"),
        (0x9401, "Humidity"),
        (0x9402, "Pressure"),
        (0x9403, "WaterDepth"),
        (0x9404, "Acceleration"),
        (0x9405, "CameraElevationAngle"),
        (0x9c9b, "XPTitle"),
        (0x9c9c, "XPComment"),
        (0x9c9d, "XPAuthor"),
        (0x9c9e, "XPKeywords"),
        (0x9c9f, "XPSubject"),
        (0xa000, "FlashpixVersion"),
        (0xa001, "ColorSpace"),
        (0xa002, "ExifImageWidth"),
        (0xa003, "ExifImageHeight"),
        (0xa004, "RelatedSoundFile"),
        (0xa005, "InteropOffset"),
        (0xa010, "SamsungRawPointersOffset"),
        (0xa011, "SamsungRawPointersLength"),
        (0xa101, "SamsungRawByteOrder"),
        (0xa102, "SamsungRawUnknown"),
        (0xa20b, "FlashEnergy"),
        (0xa20c, "SpatialFrequencyResponse"),
        (0xa20d, "Noise"),
        (0xa20e, "FocalPlaneXResolution"),
        (0xa20f, "FocalPlaneYResolution"),
        (0xa210, "FocalPlaneResolutionUnit"),
        (0xa211, "ImageNumber"),
        (0xa212, "SecurityClassification"),
        (0xa213, "ImageHistory"),
        (0xa214, "SubjectLocation"),
        (0xa215, "ExposureIndex"),
        (0xa216, "TIFF-EPStandardID"),
        (0xa217, "SensingMethod"),
        (0xa300, "FileSource"),
        (0xa301, "SceneType"),
        (0xa302, "CFAPattern"),
        (0xa401, "CustomRendered"),
        (0xa402, "ExposureMode"),
        (0xa403, "WhiteBalance"),
        (0xa404, "DigitalZoomRatio"),
        (0xa405, "FocalLengthIn35mmFormat"),
        (0xa406, "SceneCaptureType"),
        (0xa407, "GainControl"),
        (0xa408, "Contrast"),
        (0xa409, "Saturation"),
        (0xa40a, "Sharpness"),
        (0xa40b, "DeviceSettingDescription"),
        (0xa40c, "SubjectDistanceRange"),
        (0xa420, "ImageUniqueID"),
        (0xa430, "OwnerName"),
        (0xa431, "SerialNumber"),
        (0xa432, "LensInfo"),
        (0xa433, "LensMake"),
        (0xa434, "LensModel"),
        (0xa435, "LensSerialNumber"),
        (0xa460, "CompositeImage"),
        (0xa461, "CompositeImageCount"),
        (0xa462, "CompositeImageExposureTimes"),
        (0xa480, "GDALMetadata"),
        (0xa481, "GDALNoData"),
        (0xa500, "Gamma"),
        (0xafc0, "ExpandSoftware"),
        (0xafc1, "ExpandLens"),
        (0xafc2, "ExpandFilm"),
        (0xafc3, "ExpandFilterLens"),
        (0xafc4, "ExpandScanner"),
        (0xafc5, "ExpandFlashLamp"),
        (0xb4c3, "HasselbladRawImage"),
        (0xbc01, "PixelFormat"),
        (0xbc02, "Transformation"),
        (0xbc03, "Uncompressed"),
        (0xbc04, "ImageType"),
        (0xbc80, "ImageWidth"),
        (0xbc81, "ImageHeight"),
        (0xbc82, "WidthResolution"),
        (0xbc83, "HeightResolution"),
        (0xbcc0, "ImageOffset"),
        (0xbcc1, "ImageByteCount"),
        (0xbcc2, "AlphaOffset"),
        (0xbcc3, "AlphaByteCount"),
        (0xbcc4, "ImageDataDiscard"),
        (0xbcc5, "AlphaDataDiscard"),
        (0xc427, "OceScanjobDesc"),
        (0xc428, "OceApplicationSelector"),
        (0xc429, "OceIDNumber"),
        (0xc42a, "OceImageLogic"),
        (0xc44f, "Annotations"),
        (0xc4a5, "PrintIM"),
        (0xc51b, "HasselbladExif"),
        (0xc573, "OriginalFileName"),
        (0xc580, "USPTOOriginalContentType"),
        (0xc5e0, "CR2CFAPattern"),
        (0xc612, "DNGVersion"),
        (0xc613, "DNGBackwardVersion"),
        (0xc614, "UniqueCameraModel"),
        (0xc615, "LocalizedCameraModel"),
        (0xc616, "CFAPlaneColor"),
        (0xc617, "CFALayout"),
        (0xc618, "LinearizationTable"),
        (0xc619, "BlackLevelRepeatDim"),
        (0xc61a, "BlackLevel"),
        (0xc61b, "BlackLevelDeltaH"),
        (0xc61c, "BlackLevelDeltaV"),
        (0xc61d, "WhiteLevel"),
        (0xc61e, "DefaultScale"),
        (0xc61f, "DefaultCropOrigin"),
        (0xc620, "DefaultCropSize"),
        (0xc621, "ColorMatrix1"),
        (0xc622, "ColorMatrix2"),
        (0xc623, "CameraCalibration1"),
        (0xc624, "CameraCalibration2"),
        (0xc625, "ReductionMatrix1"),
        (0xc626, "ReductionMatrix2"),
        (0xc627, "AnalogBalance"),
        (0xc628, "AsShotNeutral"),
        (0xc629, "AsShotWhiteXY"),
        (0xc62a, "BaselineExposure"),
        (0xc62b, "BaselineNoise"),
        (0xc62c, "BaselineSharpness"),
        (0xc62d, "BayerGreenSplit"),
        (0xc62e, "LinearResponseLimit"),
        (0xc62f, "CameraSerialNumber"),
        (0xc630, "DNGLensInfo"),
        (0xc631, "ChromaBlurRadius"),
        (0xc632, "AntiAliasStrength"),
        (0xc633, "ShadowScale"),
        (0xc634, "SR2Private"),
        (0xc635, "MakerNoteSafety"),
        (0xc640, "RawImageSegmentation"),
        (0xc65a, "CalibrationIlluminant1"),
        (0xc65b, "CalibrationIlluminant2"),
        (0xc65c, "BestQualityScale"),
        (0xc65d, "RawDataUniqueID"),
        (0xc660, "AliasLayerMetadata"),
        (0xc68b, "OriginalRawFileName"),
        (0xc68c, "OriginalRawFileData"),
        (0xc68d, "ActiveArea"),
        (0xc68e, "MaskedAreas"),
        (0xc68f, "AsShotICCProfile"),
        (0xc690, "AsShotPreProfileMatrix"),
        (0xc691, "CurrentICCProfile"),
        (0xc692, "CurrentPreProfileMatrix"),
        (0xc6bf, "ColorimetricReference"),
        (0xc6c5, "SRawType"),
        (0xc6d2, "PanasonicTitle"),
        (0xc6d3, "PanasonicTitle2"),
        (0xc6f3, "CameraCalibrationSig"),
        (0xc6f4, "ProfileCalibrationSig"),
        (0xc6f5, "ProfileIFD"),
        (0xc6f6, "AsShotProfileName"),
        (0xc6f7, "NoiseReductionApplied"),
        (0xc6f8, "ProfileName"),
        (0xc6f9, "ProfileHueSatMapDims"),
        (0xc6fa, "ProfileHueSatMapData1"),
        (0xc6fb, "ProfileHueSatMapData2"),
        (0xc6fc, "ProfileToneCurve"),
        (0xc6fd, "ProfileEmbedPolicy"),
        (0xc6fe, "ProfileCopyright"),
        (0xc714, "ForwardMatrix1"),
        (0xc715, "ForwardMatrix2"),
        (0xc716, "PreviewApplicationName"),
        (0xc717, "PreviewApplicationVersion"),
        (0xc718, "PreviewSettingsName"),
        (0xc719, "PreviewSettingsDigest"),
        (0xc71a, "PreviewColorSpace"),
        (0xc71b, "PreviewDateTime"),
        (0xc71c, "RawImageDigest"),
        (0xc71d, "OriginalRawFileDigest"),
        (0xc71e, "SubTileBlockSize"),
        (0xc71f, "RowInterleaveFactor"),
        (0xc725, "ProfileLookTableDims"),
        (0xc726, "ProfileLookTableData"),
        (0xc740, "OpcodeList1"),
        (0xc741, "OpcodeList2"),
        (0xc74e, "OpcodeList3"),
        (0xc761, "NoiseProfile"),
        (0xc763, "TimeCodes"),
        (0xc764, "FrameRate"),
        (0xc772, "TStop"),
        (0xc789, "ReelName"),
        (0xc791, "OriginalDefaultFinalSize"),
        (0xc792, "OriginalBestQualitySize"),
        (0xc793, "OriginalDefaultCropSize"),
        (0xc7a1, "CameraLabel"),
        (0xc7a3, "ProfileHueSatMapEncoding"),
        (0xc7a4, "ProfileLookTableEncoding"),
        (0xc7a5, "BaselineExposureOffset"),
        (0xc7a6, "DefaultBlackRender"),
        (0xc7a7, "NewRawImageDigest"),
        (0xc7a8, "RawToPreviewGain"),
        (0xc7aa, "CacheVersion"),
        (0xc7b5, "DefaultUserCrop"),
        (0xc7d5, "NikonNEFInfo"),
        (0xc7e9, "DepthFormat"),
        (0xc7ea, "DepthNear"),
        (0xc7eb, "DepthFar"),
        (0xc7ec, "DepthUnits"),
        (0xc7ed, "DepthMeasureType"),
        (0xc7ee, "EnhanceParams"),
        (0xea1c, "Padding"),
        (0xea1d, "OffsetSchema"),
        (0xfde8, "OwnerName"),
        (0xfde9, "SerialNumber"),
        (0xfdea, "Lens"),
        (0xfe00, "KDC_IFD"),
        (0xfe4c, "RawFile"),
        (0xfe4d, "Converter"),
        (0xfe4e, "WhiteBalance"),
        (0xfe51, "Exposure"),
        (0xfe52, "Shadows"),
        (0xfe53, "Brightness"),
        (0xfe54, "Contrast"),
        (0xfe55, "Saturation"),
        (0xfe56, "Sharpness"),
        (0xfe57, "Smoothness"),
        (0xfe58, "MoireFilter"),
    ]);
}
