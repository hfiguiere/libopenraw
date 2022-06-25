/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* Generated with cbindgen:0.24.3 */

/* DO NOT MODIFY THIS MANUALLY! This file was generated using cbindgen. */
#ifndef mp4parse_rust_mp4parse_h
#error "Don't include this file directly, instead include mp4parse.h"
#endif


#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * The axis about which the image is mirrored (opposite of flip)
 * Visualized in terms of starting with (⥠) UPWARDS HARPOON WITH BARB LEFT FROM BAR
 * similar to a DIGIT ONE (1)
 */
typedef enum Mp4parseImir {
  /**
   * top and bottom parts exchanged
   * ⥡ DOWNWARDS HARPOON WITH BARB LEFT FROM BAR
   */
  MP4PARSE_IMIR_TOP_BOTTOM,
  /**
   * left and right parts exchanged
   * ⥜ UPWARDS HARPOON WITH BARB RIGHT FROM BAR
   */
  MP4PARSE_IMIR_LEFT_RIGHT,
} Mp4parseImir;

/**
 * Rotation in the positive (that is, anticlockwise) direction
 * Visualized in terms of starting with (⥠) UPWARDS HARPOON WITH BARB LEFT FROM BAR
 * similar to a DIGIT ONE (1)
 */
typedef enum Mp4parseIrot {
  /**
   * ⥠ UPWARDS HARPOON WITH BARB LEFT FROM BAR
   */
  MP4PARSE_IROT_D0,
  /**
   * ⥞ LEFTWARDS HARPOON WITH BARB DOWN FROM BAR
   */
  MP4PARSE_IROT_D90,
  /**
   * ⥝ DOWNWARDS HARPOON WITH BARB RIGHT FROM BAR
   */
  MP4PARSE_IROT_D180,
  /**
   * ⥛  RIGHTWARDS HARPOON WITH BARB UP FROM BAR
   */
  MP4PARSE_IROT_D270,
} Mp4parseIrot;

typedef enum Mp4ParseEncryptionSchemeType {
  MP4_PARSE_ENCRYPTION_SCHEME_TYPE_NONE,
  MP4_PARSE_ENCRYPTION_SCHEME_TYPE_CENC,
  MP4_PARSE_ENCRYPTION_SCHEME_TYPE_CBC1,
  MP4_PARSE_ENCRYPTION_SCHEME_TYPE_CENS,
  MP4_PARSE_ENCRYPTION_SCHEME_TYPE_CBCS,
} Mp4ParseEncryptionSchemeType;

typedef enum Mp4parseCodec {
  MP4PARSE_CODEC_UNKNOWN,
  MP4PARSE_CODEC_AAC,
  MP4PARSE_CODEC_FLAC,
  MP4PARSE_CODEC_OPUS,
  MP4PARSE_CODEC_AVC,
  MP4PARSE_CODEC_VP9,
  MP4PARSE_CODEC_AV1,
  MP4PARSE_CODEC_MP3,
  MP4PARSE_CODEC_MP4V,
  MP4PARSE_CODEC_JPEG,
  MP4PARSE_CODEC_AC3,
  MP4PARSE_CODEC_EC3,
  MP4PARSE_CODEC_ALAC,
  MP4PARSE_CODEC_H263,
#if defined(MP4PARSE_FEATURE_3GPP)
  MP4PARSE_CODEC_AMRNB,
#endif
#if defined(MP4PARSE_FEATURE_3GPP)
  MP4PARSE_CODEC_AMRWB,
#endif
  MP4PARSE_CODEC_CRAW,
} Mp4parseCodec;

typedef enum Mp4parseTrackType {
  MP4PARSE_TRACK_TYPE_VIDEO = 0,
  MP4PARSE_TRACK_TYPE_AUDIO = 1,
  MP4PARSE_TRACK_TYPE_METADATA = 2,
} Mp4parseTrackType;

typedef enum Mp4parseStrictness {
  MP4PARSE_STRICTNESS_PERMISSIVE,
  MP4PARSE_STRICTNESS_NORMAL,
  MP4PARSE_STRICTNESS_STRICT,
} Mp4parseStrictness;

/**
 * The return value to the C API
 * Any detail that needs to be communicated to the caller must be encoded here
 * since the [`Error`] type's associated data is part of the FFI.
 */
typedef enum Mp4parseStatus {
  MP4PARSE_STATUS_OK = 0,
  MP4PARSE_STATUS_BAD_ARG = 1,
  MP4PARSE_STATUS_INVALID = 2,
  MP4PARSE_STATUS_UNSUPPORTED = 3,
  MP4PARSE_STATUS_EOF = 4,
  MP4PARSE_STATUS_IO = 5,
  MP4PARSE_STATUS_OOM = 6,
  MP4PARSE_STATUS_UNSUPPORTED_A1LX,
  MP4PARSE_STATUS_UNSUPPORTED_A1OP,
  MP4PARSE_STATUS_UNSUPPORTED_CLAP,
  MP4PARSE_STATUS_UNSUPPORTED_GRID,
  MP4PARSE_STATUS_UNSUPPORTED_IPRO,
  MP4PARSE_STATUS_UNSUPPORTED_LSEL,
} Mp4parseStatus;

typedef struct Mp4parseAvifParser Mp4parseAvifParser;

typedef struct Mp4parseParser Mp4parseParser;

typedef struct Mp4parseIo {
  intptr_t (*read)(uint8_t *buffer, uintptr_t size, void *userdata);
  void *userdata;
} Mp4parseIo;

#if defined(MP4PARSE_UNSTABLE_API)
/**
 * A zero-overhead wrapper around integer types for the sake of always
 * requiring checked arithmetic
 */
typedef int64_t CheckedInteger_i64;
#endif

typedef struct Mp4parseTrackInfo {
  enum Mp4parseTrackType track_type;
  uint32_t track_id;
  uint64_t duration;
  CheckedInteger_i64 media_time;
} Mp4parseTrackInfo;

#if defined(MP4PARSE_UNSTABLE_API)
/**
 * A zero-overhead wrapper around integer types for the sake of always
 * requiring checked arithmetic
 */
typedef uint64_t CheckedInteger_u64;
#endif

#if defined(MP4PARSE_UNSTABLE_API)
/**
 * Provides the following information about a sample in the source file:
 * sample data offset (start and end), composition time in microseconds
 * (start and end) and whether it is a sync sample
 */
typedef struct Mp4parseIndice {
  /**
   * The byte offset in the file where the indexed sample begins.
   */
  CheckedInteger_u64 start_offset;
  /**
   * The byte offset in the file where the indexed sample ends. This is
   * equivalent to `start_offset` + the length in bytes of the indexed
   * sample. Typically this will be the `start_offset` of the next sample
   * in the file.
   */
  CheckedInteger_u64 end_offset;
  /**
   * The time in microseconds when the indexed sample should be displayed.
   * Analogous to the concept of presentation time stamp (pts).
   */
  CheckedInteger_i64 start_composition;
  /**
   * The time in microseconds when the indexed sample should stop being
   * displayed. Typically this would be the `start_composition` time of the
   * next sample if samples were ordered by composition time.
   */
  CheckedInteger_i64 end_composition;
  /**
   * The time in microseconds that the indexed sample should be decoded at.
   * Analogous to the concept of decode time stamp (dts).
   */
  CheckedInteger_i64 start_decode;
  /**
   * Set if the indexed sample is a sync sample. The meaning of sync is
   * somewhat codec specific, but essentially amounts to if the sample is a
   * key frame.
   */
  bool sync;
} Mp4parseIndice;
#endif

typedef struct Mp4parseByteData {
  uintptr_t length;
  const uint8_t *data;
  const struct Mp4parseIndice *indices;
} Mp4parseByteData;

enum OptionalFourCc_Tag
#ifdef __cplusplus
  : uint8_t
#endif // __cplusplus
 {
  OPTIONAL_FOUR_CC_NONE,
  OPTIONAL_FOUR_CC_SOME,
};
#ifndef __cplusplus
typedef uint8_t OptionalFourCc_Tag;
#endif // __cplusplus

typedef union OptionalFourCc {
  OptionalFourCc_Tag tag;
  struct {
    OptionalFourCc_Tag some_tag;
    uint8_t some[4];
  };
} OptionalFourCc;

typedef struct Mp4parseSinfInfo {
  union OptionalFourCc original_format;
  enum Mp4ParseEncryptionSchemeType scheme_type;
  uint8_t is_encrypted;
  uint8_t iv_size;
  struct Mp4parseByteData kid;
  uint8_t crypt_byte_block;
  uint8_t skip_byte_block;
  struct Mp4parseByteData constant_iv;
} Mp4parseSinfInfo;

typedef struct Mp4parseTrackAudioSampleInfo {
  enum Mp4parseCodec codec_type;
  uint16_t channels;
  uint16_t bit_depth;
  uint32_t sample_rate;
  uint16_t profile;
  uint16_t extended_profile;
  struct Mp4parseByteData codec_specific_config;
  struct Mp4parseByteData extra_data;
  struct Mp4parseSinfInfo protected_data;
} Mp4parseTrackAudioSampleInfo;

typedef struct Mp4parseTrackAudioInfo {
  uint32_t sample_info_count;
  const struct Mp4parseTrackAudioSampleInfo *sample_info;
} Mp4parseTrackAudioInfo;

typedef struct Mp4parseTrackRawInfo {
  uint16_t image_width;
  uint16_t image_height;
  bool is_jpeg;
  uint64_t offset;
  uint64_t size;
} Mp4parseTrackRawInfo;

typedef struct Mp4parseTrackVideoSampleInfo {
  enum Mp4parseCodec codec_type;
  uint16_t image_width;
  uint16_t image_height;
  struct Mp4parseByteData extra_data;
  struct Mp4parseSinfInfo protected_data;
} Mp4parseTrackVideoSampleInfo;

typedef struct Mp4parseTrackVideoInfo {
  uint32_t display_width;
  uint32_t display_height;
  uint16_t rotation;
  uint32_t sample_info_count;
  const struct Mp4parseTrackVideoSampleInfo *sample_info;
} Mp4parseTrackVideoInfo;

typedef struct Mp4parseAvifImageItem {
  struct Mp4parseByteData coded_data;
  struct Mp4parseByteData bits_per_channel;
} Mp4parseAvifImageItem;

typedef struct Mp4parseImageSpatialExtents {
  uint32_t image_width;
  uint32_t image_height;
} Mp4parseImageSpatialExtents;

/**
 * Despite [Rec. ITU-T H.273] (12/2016) defining the CICP fields as having a
 * range of 0-255, and only a small fraction of those values being used,
 * ISOBMFF (ISO 14496-12:2020) § 12.1.5 defines them as 16-bit values in the
 * `colr` box. Since we have no use for the additional range, and it would
 * complicate matters later, we fallibly convert before storing the input.
 *
 * [Rec. ITU-T H.273]: https://www.itu.int/rec/T-REC-H.273-201612-I/en
 */
typedef struct Mp4parseNclxColourInformation {
  uint8_t colour_primaries;
  uint8_t transfer_characteristics;
  uint8_t matrix_coefficients;
  bool full_range_flag;
} Mp4parseNclxColourInformation;

typedef struct Mp4parseAvifImage {
  struct Mp4parseAvifImageItem primary_image;
  /**
   * The size of the image; should never be null unless using permissive parsing
   */
  const struct Mp4parseImageSpatialExtents *spatial_extents;
  const struct Mp4parseNclxColourInformation *nclx_colour_information;
  struct Mp4parseByteData icc_colour_information;
  enum Mp4parseIrot image_rotation;
  const enum Mp4parseImir *image_mirror;
  /**
   * If no alpha item exists, members' `.length` will be 0 and `.data` will be null
   */
  struct Mp4parseAvifImageItem alpha_image;
  bool premultiplied_alpha;
} Mp4parseAvifImage;

typedef struct Mp4parseFragmentInfo {
  uint64_t fragment_duration;
} Mp4parseFragmentInfo;

typedef struct Mp4parsePsshInfo {
  struct Mp4parseByteData data;
} Mp4parsePsshInfo;

typedef struct Mp4parseCrawHeader {
  struct Mp4parseByteData cncv;
  uint16_t thumb_w;
  uint16_t thumb_h;
  struct Mp4parseByteData thumbnail;
  struct Mp4parseByteData meta1;
  struct Mp4parseByteData meta2;
  struct Mp4parseByteData meta3;
  struct Mp4parseByteData meta4;
} Mp4parseCrawHeader;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Allocate an `Mp4parseParser*` to read from the supplied `Mp4parseIo` and
 * parse the content from the `Mp4parseIo` argument until EOF or error.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the `io` and `parser_out`
 * pointers given to it. The caller should ensure that the `Mp4ParseIo`
 * struct passed in is a valid pointer. The caller should also ensure the
 * members of io are valid: the `read` function should be sanely implemented,
 * and the `userdata` pointer should be valid. The `parser_out` should be a
 * valid pointer to a location containing a null pointer. Upon successful
 * return (`Mp4parseStatus::Ok`), that location will contain the address of
 * an `Mp4parseParser` allocated by this function.
 *
 * To avoid leaking memory, any successful return of this function must be
 * paired with a call to `mp4parse_free`. In the event of error, no memory
 * will be allocated and `mp4parse_free` must *not* be called.
 */
Mp4parseStatus mp4parse_new(const struct Mp4parseIo *io, struct Mp4parseParser **parser_out);

/**
 * Allocate an `Mp4parseAvifParser*` to read from the supplied `Mp4parseIo`.
 *
 * See mp4parse_new; this function is identical except that it allocates an
 * `Mp4parseAvifParser`, which (when successful) must be paired with a call
 * to mp4parse_avif_free.
 *
 * # Safety
 *
 * Same as mp4parse_new.
 */
Mp4parseStatus mp4parse_avif_new(const struct Mp4parseIo *io,
                                 enum Mp4parseStrictness strictness,
                                 struct Mp4parseAvifParser **parser_out);

/**
 * Free an `Mp4parseParser*` allocated by `mp4parse_new()`.
 *
 * # Safety
 *
 * This function is unsafe because it creates a box from a raw pointer.
 * Callers should ensure that the parser pointer points to a valid
 * `Mp4parseParser` created by `mp4parse_new`.
 */
void mp4parse_free(struct Mp4parseParser *parser);

/**
 * Free an `Mp4parseAvifParser*` allocated by `mp4parse_avif_new()`.
 *
 * # Safety
 *
 * This function is unsafe because it creates a box from a raw pointer.
 * Callers should ensure that the parser pointer points to a valid
 * `Mp4parseAvifParser` created by `mp4parse_avif_new`.
 */
void mp4parse_avif_free(struct Mp4parseAvifParser *parser);

/**
 * Return the number of tracks parsed by previous `mp4parse_read()` call.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences both the parser and count
 * raw pointers passed into it. Callers should ensure the parser pointer
 * points to a valid `Mp4parseParser`, and that the count pointer points an
 * appropriate memory location to have a `u32` written to.
 */
Mp4parseStatus mp4parse_get_track_count(const struct Mp4parseParser *parser, uint32_t *count);

/**
 * Fill the supplied `Mp4parseTrackInfo` with metadata for `track`.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and info raw
 * pointers passed to it. Callers should ensure the parser pointer points to a
 * valid `Mp4parseParser` and that the info pointer points to a valid
 * `Mp4parseTrackInfo`.
 */
Mp4parseStatus mp4parse_get_track_info(struct Mp4parseParser *parser,
                                       uint32_t track_index,
                                       struct Mp4parseTrackInfo *info);

/**
 * Fill the supplied `Mp4parseTrackAudioInfo` with metadata for `track`.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and info raw
 * pointers passed to it. Callers should ensure the parser pointer points to a
 * valid `Mp4parseParser` and that the info pointer points to a valid
 * `Mp4parseTrackAudioInfo`.
 */
Mp4parseStatus mp4parse_get_track_audio_info(struct Mp4parseParser *parser,
                                             uint32_t track_index,
                                             struct Mp4parseTrackAudioInfo *info);

/**
 * File the supplied `Mp4parseTrackRawInfo` with metadata for `track`.
 *
 * # Safety
 * This function dereference `parser` and `info`. If they are null,
 * `Mp4parseStatus::BadArg` is returned.
 */
Mp4parseStatus mp4parse_get_track_raw_info(struct Mp4parseParser *parser,
                                           uint32_t track_index,
                                           struct Mp4parseTrackRawInfo *info);

/**
 * Fill the supplied `Mp4parseTrackVideoInfo` with metadata for `track`.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and info raw
 * pointers passed to it. Callers should ensure the parser pointer points to a
 * valid `Mp4parseParser` and that the info pointer points to a valid
 * `Mp4parseTrackVideoInfo`.
 */
Mp4parseStatus mp4parse_get_track_video_info(struct Mp4parseParser *parser,
                                             uint32_t track_index,
                                             struct Mp4parseTrackVideoInfo *info);

/**
 * Return a pointer to the primary item parsed by previous `mp4parse_avif_new()` call.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences both the parser and
 * avif_image raw pointers passed into it. Callers should ensure the parser
 * pointer points to a valid `Mp4parseAvifParser`, and that the avif_image
 * pointer points to a valid `Mp4parseAvifImage`. If there was not a previous
 * successful call to `mp4parse_avif_read()`, no guarantees are made as to
 * the state of `avif_image`. If `avif_image.alpha_image.coded_data` is set to
 * a positive `length` and non-null `data`, then the `avif_image` contains a
 * valid alpha channel data. Otherwise, the image is opaque.
 */
Mp4parseStatus mp4parse_avif_get_image(const struct Mp4parseAvifParser *parser,
                                       struct Mp4parseAvifImage *avif_image);

/**
 * Fill the supplied `Mp4parseByteData` with index information from `track`.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and indices
 * raw pointers passed to it. Callers should ensure the parser pointer points
 * to a valid `Mp4parseParser` and that the indices pointer points to a valid
 * `Mp4parseByteData`.
 */
Mp4parseStatus mp4parse_get_indice_table(struct Mp4parseParser *parser,
                                         uint32_t track_id,
                                         struct Mp4parseByteData *indices);

/**
 * Fill the supplied `Mp4parseFragmentInfo` with metadata from fragmented file.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and
 * info raw pointers passed to it. Callers should ensure the parser
 * pointer points to a valid `Mp4parseParser` and that the info pointer points
 * to a valid `Mp4parseFragmentInfo`.
 */
Mp4parseStatus mp4parse_get_fragment_info(struct Mp4parseParser *parser,
                                          struct Mp4parseFragmentInfo *info);

/**
 * Determine if an mp4 file is fragmented. A fragmented file needs mvex table
 * and contains no data in stts, stsc, and stco boxes.
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and
 * fragmented raw pointers passed to it. Callers should ensure the parser
 * pointer points to a valid `Mp4parseParser` and that the fragmented pointer
 * points to an appropriate memory location to have a `u8` written to.
 */
Mp4parseStatus mp4parse_is_fragmented(struct Mp4parseParser *parser,
                                      uint32_t track_id,
                                      uint8_t *fragmented);

/**
 * Get 'pssh' system id and 'pssh' box content for eme playback.
 *
 * The data format of the `info` struct passed to gecko is:
 *
 * - system id (16 byte uuid)
 * - pssh box size (32-bit native endian)
 * - pssh box content (including header)
 *
 * # Safety
 *
 * This function is unsafe because it dereferences the the parser and
 * info raw pointers passed to it. Callers should ensure the parser
 * pointer points to a valid `Mp4parseParser` and that the fragmented pointer
 * points to a valid `Mp4parsePsshInfo`.
 */
Mp4parseStatus mp4parse_get_pssh_info(struct Mp4parseParser *parser, struct Mp4parsePsshInfo *info);

/**
 * Get the Craw hader for the file.
 *
 * # Safety
 * This function is unsafe as it derefenece the Parser
 * and the CrawHeader
 */
Mp4parseStatus mp4parse_get_craw_header(struct Mp4parseParser *parser,
                                        struct Mp4parseCrawHeader *header);

/**
 * Get the Craw table entry
 *
 * # Safety
 * The function dereference the parser, and return offset
 * and size by pointer. They can't be null, otherwise
 * `Mp4parseStatus::BadArg` is returned.
 */
Mp4parseStatus mp4parse_get_craw_table_entry(struct Mp4parseParser *parser,
                                             uintptr_t idx,
                                             uint64_t *offset,
                                             uint64_t *size);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
