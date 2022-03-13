//! Olympus ORF support

use std::collections::HashMap;

lazy_static::lazy_static! {
    /// Olympus MakerNote tag names
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x0, "MakerNoteVersion"),
        (0x1, "MinoltaCameraSettingsOld"),
        (0x3, "MinoltaCameraSettings"),
        (0x40, "CompressedImageSize"),
        (0x81, "PreviewImageData"),
        (0x88, "PreviewImageStart"),
        (0x89, "PreviewImageLength"),
        (0x100, "ThumbnailImage"),
        (0x104, "BodyFirmwareVersion"),
        (0x200, "SpecialMode"),
        (0x201, "Quality"),
        (0x202, "Macro"),
        (0x203, "BWMode"),
        (0x204, "DigitalZoom"),
        (0x205, "FocalPlaneDiagonal"),
        (0x206, "LensDistortionParams"),
        (0x207, "CameraType"),
        (0x208, "TextInfo"),
        (0x209, "CameraID"),
        (0x20b, "EpsonImageWidth"),
        (0x20c, "EpsonImageHeight"),
        (0x20d, "EpsonSoftware"),
        (0x280, "PreviewImage"),
        (0x300, "PreCaptureFrames"),
        (0x301, "WhiteBoard"),
        (0x302, "OneTouchWB"),
        (0x303, "WhiteBalanceBracket"),
        (0x304, "WhiteBalanceBias"),
        (0x400, "SensorArea"),
        (0x401, "BlackLevel"),
        (0x403, "SceneMode"),
        (0x404, "SerialNumber"),
        (0x405, "Firmware"),
        (0xe00, "PrintIM"),
        (0xf00, "DataDump"),
        (0xf01, "DataDump2"),
        (0xf04, "ZoomedPreviewStart"),
        (0xf05, "ZoomedPreviewLength"),
        (0xf06, "ZoomedPreviewSize"),
        (0x1000, "ShutterSpeedValue"),
        (0x1001, "ISOValue"),
        (0x1002, "ApertureValue"),
        (0x1003, "BrightnessValue"),
        (0x1004, "FlashMode"),
        (0x1005, "FlashDevice"),
        (0x1006, "ExposureCompensation"),
        (0x1007, "SensorTemperature"),
        (0x1008, "LensTemperature"),
        (0x1009, "LightCondition"),
        (0x100a, "FocusRange"),
        (0x100b, "FocusMode"),
        (0x100c, "ManualFocusDistance"),
        (0x100d, "ZoomStepCount"),
        (0x100e, "FocusStepCount"),
        (0x100f, "Sharpness"),
        (0x1010, "FlashChargeLevel"),
        (0x1011, "ColorMatrix"),
        (0x1012, "BlackLevel"),
        (0x1013, "ColorTemperatureBG"),
        (0x1014, "ColorTemperatureRG"),
        (0x1015, "WBMode"),
        (0x1017, "RedBalance"),
        (0x1018, "BlueBalance"),
        (0x1019, "ColorMatrixNumber"),
        (0x101a, "SerialNumber"),
        (0x101b, "ExternalFlashAE1_0"),
        (0x101c, "ExternalFlashAE2_0"),
        (0x101d, "InternalFlashAE1_0"),
        (0x101e, "InternalFlashAE2_0"),
        (0x101f, "ExternalFlashAE1"),
        (0x1020, "ExternalFlashAE2"),
        (0x1021, "InternalFlashAE1"),
        (0x1022, "InternalFlashAE2"),
        (0x1023, "FlashExposureComp"),
        (0x1024, "InternalFlashTable"),
        (0x1025, "ExternalFlashGValue"),
        (0x1026, "ExternalFlashBounce"),
        (0x1027, "ExternalFlashZoom"),
        (0x1028, "ExternalFlashMode"),
        (0x1029, "Contrast"),
        (0x102a, "SharpnessFactor"),
        (0x102b, "ColorControl"),
        (0x102c, "ValidBits"),
        (0x102d, "CoringFilter"),
        (0x102e, "OlympusImageWidth"),
        (0x102f, "OlympusImageHeight"),
        (0x1030, "SceneDetect"),
        (0x1031, "SceneArea"),
        (0x1033, "SceneDetectData"),
        (0x1034, "CompressionRatio"),
        (0x1035, "PreviewImageValid"),
        (0x1036, "PreviewImageStart"),
        (0x1037, "PreviewImageLength"),
        (0x1038, "AFResult"),
        (0x1039, "CCDScanMode"),
        (0x103a, "NoiseReduction"),
        (0x103b, "FocusStepInfinity"),
        (0x103c, "FocusStepNear"),
        (0x103d, "LightValueCenter"),
        (0x103e, "LightValuePeriphery"),
        (0x103f, "FieldCount"),
        (0x2010, "Equipment"),
        (0x2020, "CameraSettings"),
        (0x2030, "RawDevelopment"),
        (0x2031, "RawDev2"),
        (0x2040, "ImageProcessing"),
        (0x2050, "FocusInfo"),
        (0x2100, "Olympus2100"),
        (0x2200, "Olympus2200"),
        (0x2300, "Olympus2300"),
        (0x2400, "Olympus2400"),
        (0x2500, "Olympus2500"),
        (0x2600, "Olympus2600"),
        (0x2700, "Olympus2700"),
        (0x2800, "Olympus2800"),
        (0x2900, "Olympus2900"),
        (0x3000, "RawInfo"),
        (0x4000, "MainInfo"),
        (0x5000, "UnknownInfo"),
    ]);
}
