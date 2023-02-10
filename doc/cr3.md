Canon CR3

Appeared in 2018 with the EOS M 50.
Also related to CRM from Canon C-series cinema camera.

Most of this info comes from <https://github.com/lclevy/canon_cr3>

Container
=========

The CR3 is basically an ISO Base Media container (MPEG4 Part 12, ISO
14496-12) like `.mp4` files, but with a certain number a differences in
its content.

Here the term of box has the same definition as in the ISO Base Media
specification.

Overview
--------

The brand is `crx ` instead of `isom`.

The movie box (`moov`) starts with a `uuid` box, the header. This box
contains the Exif matadata, including the MakerNote and an Exif size
thumbnail (JPEG, 160x120).

It also contain a table `CTBO` for the main data blobs offset and length:
- an XMP block in a `uuid` box.
- a smaller JPEG preview in a uuid box.
- data which contain a JPEG render, a preview, the main RAW data
  and the metadata.
This is probably a shortcut to avoid having to parse the whole ISO
Base Media container.

The movie box also contain 4 tracks: 3 video, 1 metadata for the JPEG
render, the preview, the main RAW data and the metadata. All point to
the media data box (`mdat`). They have one chunk defined.

After the movie box, there are two `uuid` boxes, that are pointed to
by the table `CTBO`, the first box is for the XMP packet, the other is
for the preview.

Trailing is the media data box "mdat".

# Description of the format.

```text
+ 'xxxx'
```

This represent a box with type 'xxxx' as defined in the ISO Base
Media. Data is in Big-Endian.

```text
| (type) # description
```

This represent a data of type, and what its description. It is
contained in the box defined on the same level.

Box are contained in other boxes.

```text
+ 'xxxx'
| (u32) 00 00 00 00 # some data
| + 'yyyy'
| | (String) # some string
+ 'zzzz'
```

Box 'xxxx' contains an u32 data value that as byte value 00 00 00 00,
and then contains box 'yyyy'. Box 'zzzz' follows box 'xxxx'.

If the data value has an expected value, it is indicated, either as
raw data or as its value.

Datatype are:

- UUID: 16 byte UUID.
- String: a string. Size is determined by the box size unless
  indicated oterwise.
- u8: a single byte.
- u16: 16 bits unsigned (big endian).
- u32: 32 bits unsigned (big endian).
- IFDFile: data blob formatted like an IFD file (TIFF), similar in use
  to Exif.
- * n: data is repeated n times (with n being a literal or another
  variable).

# The header

The header is a `uuid` with UUID `85c0b687 820f 11e0 8111 f4ce462b6a48`,
it is contained by the movie box.

The structure is as follow:

```text
+ 'uuid'
| (UUID) 85c0b687 820f 11e0 8111 f4ce462b6a48
| + 'CNCV'
| | (String) # Seems to define the compressor version
| + 'CCTP'
| | + 'CCDT'
| | + 'CCDT'
| | + 'CCDT'
| + 'CTBO'
| | (CTBO Table) # offsets to other boxes
| + 'free'
| + 'CMT1'
| | (IFDFile) # equivalent of IFD 0 (main directory)
| + 'CMT2'
| | (IFDFile) # Exif IFD
| + 'CMT3'
| | (IFDFile) # Canon MakeNote
| + 'CMT4'
| | (IFDFile) # GPS IFD
| + 'THMB'
| | (u32) 0
| | (u16) # width
| | (u16) # height
| | (u32) # byte_length of the JPEG
| | (u32) 00 01 00 00
| | (u8 * byte_length) # JPEG data stream of byte_length
```

# Tracks

All four tracks have in common is that the sampte description box
(`stsd`) contains a `CRAW` box, which is specific to Canon.

These CRAW box mostly conform to the `VisualSampleEntry` from the ISO
Base Media specification.

```text
+ 'stsd'
| + 'CRAW'
| | (u32)
| | (u16)
| | (u16) 1 # data reference index
| | (i16) 0 # pre_defined
| | (u16) 0 # reserved
| | (u32 * 3) 0 # pre_defined
| | (u16) # width
| | (u16) # height
| | (u32) 0x00480000 # x resolution 72 dpi fixed 16.16
| | (u32) 0x00480000 # y resolution 72 dpi fixed 16.16
| | (u32) 0 # reserved
| | (String)[32] "\0" # compressor name?
| | (u32) 24 # depth
| | (i16) -1 # pre_defined
| | (u16) 3
| | (u16) 0
[...]
```

## Track 1

Track 1 of type video is for the JPEG render of the image.

```text
| + 'CRAW'
| | [...] # see above for the fields
| | + 'JPEG'
| | | (u32) 0 #
| | + 'free'
| | | (u16) 0 #
```

## Track 2 & 3

Track 2 of type video is for the small RGB preview.
Track 3 of type video is for the RAW image.

These two track are very similar in content.

```text
| + 'CRAW'
| | [...] # see above for the fields
| | + 'CMP1'
| | | (u32) 0 #
| | + 'CDI1'
| | | + 'IAD1'
| | | | (u32) 0
| | | | (u16) # width
| | | | (u16) # height
| | | | (u16) 1
| | | | (u16) 0
| | | | (u16) 1
| | | | (u32) 1
| | | | (u16) 0
| | | | (u16) # w?
| | | | (u16) # h?
| | | | (u32) 0
| | | | (u16) # w?
| | | | (u16) # h?
| | + 'free'
| | | (u16) 0 #

```

## Track 4

Track 4 of type metadata is for the RAW metadata

# The XMP box

The XMP box is a `uuid` with UUID `be7acfcb 97a9 42e8 9c71
999491e3afac`, at the top level of the container.

```text
+ 'uuid'
| (UUID) be7acfcb 97a9 42e8 9c71 999491e3afac
| (String) # The XMP packet wrapper.
```

Its content is an XMP xpacket wrapper with enough padding that it
should be possible to rewrite the XMP without changing anything else
in the file.

After this many year, it is nice to see Canon acknowledging the need
for XMP support.

# The Preview box

The preview box is a `uuid` with UUID `eaf42b5e 1c98 4b88 b9fb
b7dc406e4d16`, at the top level of the container.

```text
+ 'uuid'
| (UUID) eaf42b5e 1c98 4b88 b9fb b7dc406e4d16
| (u32)
| (u32)
| + 'PRVW'
| | (u32) 0
| | (u16) 1
| | (u16) # width
| | (u16) # height
| | (u16) 1
| | (u32) # byte_length
| | (u8 * byte_length) # the JPEG data stream
```
