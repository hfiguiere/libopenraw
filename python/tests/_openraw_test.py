#!/usr/bin/env python

# Copyright (C) 2008 Brian Quinlan
#
# This library is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library.  If not, see
# <http://www.gnu.org/licenses/>.

"""Tests the low-level _openraw module."""

__author__ = 'Brian Quinlan <brian@sweetapp.com>'

from openraw import _openraw
import unittest

RAW_EXAMPLE1_PATH = './images/duck.cr2'


class RawFile_newRawFile_TestCase(unittest.TestCase):
    """Tests _openraw.RawFile.newRawFile."""

    def test_newRawFile(self):
        rawfile = _openraw.RawFile.newRawFile(
            RAW_EXAMPLE1_PATH,
            _openraw.Type.OR_RAWFILE_TYPE_UNKNOWN)

        self.assert_(rawfile is not None)

    def test_newRawFile_with_unguessable_type(self):
        rawfile = _openraw.RawFile.newRawFile(
            '<no extension>',
            _openraw.Type.OR_RAWFILE_TYPE_UNKNOWN)

        self.assert_(rawfile is None)

    def test_newRawFile_with_forced_type(self):
        rawfile = _openraw.RawFile.newRawFile(
            '<no extension>',
            _openraw.Type.OR_RAWFILE_TYPE_CR2)

        self.assert_(rawfile is not None)
        self.assertEqual(_openraw.Type.OR_RAWFILE_TYPE_CR2, rawfile.type())


class RawFile_newRawFileFromMemory_TestCase(unittest.TestCase):
    """Tests _openraw.RawFile.newRawFileFromMemory."""

    def test_newRawFileFromMemory(self):
        data = open(RAW_EXAMPLE1_PATH).read()
        rawfile = _openraw.RawFile.newRawFileFromMemory(
            data,
            _openraw.Type.OR_RAWFILE_TYPE_UNKNOWN)

        self.assert_(rawfile is not None)
        self.assertEqual([1936, 160, 486], rawfile.listThumbnailSizes())


class RawFileMethodsTestCase(unittest.TestCase):
    """Tests _openraw.RawFile instance methods."""

    def setUp(self):
        self.rawfile1 = _openraw.RawFile.newRawFile(
            RAW_EXAMPLE1_PATH,
            _openraw.Type.OR_RAWFILE_TYPE_UNKNOWN)

    def test_type(self):
        self.assertEqual(_openraw.Type.OR_RAWFILE_TYPE_CR2,
                         self.rawfile1.type())

    def test_listThumbnailSizes(self):
        self.assertEqual([1936, 160, 486], self.rawfile1.listThumbnailSizes())

    def test_getThumbnail(self):
        thumbnail = _openraw.Thumbnail()
        self.assertEqual(_openraw.or_error.OR_ERROR_NONE,
                         self.rawfile1.getThumbnail(1024, thumbnail))
        self.assertEqual(408021, thumbnail.size())

    def test_getThumbnail_with_none(self):
        self.assertRaises(TypeError,
                         self.rawfile1.getThumbnail,
                         1024,
                         None)

    def test_getOrientation(self):
        self.assertEqual(1, self.rawfile1.getOrientation())

    def test_getRawData(self):
        rawdata = _openraw.RawData()

        self.assertEqual(
            _openraw.or_error.OR_ERROR_NONE,
            self.rawfile1.getRawData(rawdata,
                                     _openraw.or_options.OR_OPTIONS_NONE))
        self.assertEqual(20682336, rawdata.size())


class RawDataMethodsTestCase(unittest.TestCase):
    """Tests _openraw.RawData instance methods."""

    def setUp(self):
        rawfile1 = _openraw.RawFile.newRawFile(
            RAW_EXAMPLE1_PATH,
            _openraw.Type.OR_RAWFILE_TYPE_UNKNOWN)
        self.rawdata1 = _openraw.RawData()
        rawfile1.getRawData(self.rawdata1, _openraw.or_options.OR_OPTIONS_NONE)

    def test_min(self):
        self.assertEqual(0, self.rawdata1.min())

    def test_setMin(self):
        self.rawdata1.setMin(5)
        self.assertEqual(5, self.rawdata1.min())

    def test_max(self):
        self.assertEqual(16383, self.rawdata1.max())

    def test_setMax(self):
        self.rawdata1.setMax(5)
        self.assertEqual(5, self.rawdata1.max())
 
    def test_compression(self):
        self.assertEqual(0, self.rawdata1.compression())

    def test_setCompression(self):
        self.rawdata1.setCompression(5)
        self.assertEqual(5, self.rawdata1.compression())

class Thumbnail_getAndExtractThumbnail_TestCase(unittest.TestCase):
    """Tests _openraw.Thumbnail.getAndExtractThumbnail."""

    def test_valid_path(self):
        thumbnail = _openraw.Thumbnail.getAndExtractThumbnail(RAW_EXAMPLE1_PATH,
                                                              0)
        self.assertEqual(8207, thumbnail.size())

    def test_invalid_path(self):
        try:
            _openraw.Thumbnail.getAndExtractThumbnail('', 0)
            self.fail("expected exception")
        except Exception, e:
            self.assertEquals((_openraw.or_error.OR_ERROR_CANT_OPEN,),
                              e.args)


class BitmapDataMethodsTestCase(unittest.TestCase):
    """Tests _openraw.Bitmap instance methods."""

    def setUp(self):
        rawfile = _openraw.RawFile.newRawFile(
            RAW_EXAMPLE1_PATH,
            _openraw.Type.OR_RAWFILE_TYPE_UNKNOWN)
        self.bitmap1 = _openraw.Thumbnail()
        rawfile.getThumbnail(160, self.bitmap1)  # OR_DATA_TYPE_JPEG

        self.bitmap2 = _openraw.Thumbnail()
        rawfile.getThumbnail(486, self.bitmap2)  # OR_DATA_TYPE_PIXMAP_8RGB

    def test_swap(self):
        data1 = self.bitmap1.data()
        data2 = self.bitmap2.data()

        self.bitmap1.swap(self.bitmap2)
        self.assertEqual(data1, self.bitmap2.data())
        self.assertEqual(data2, self.bitmap1.data())

    def test_dataType(self):
        self.assertEqual(_openraw.DataType.OR_DATA_TYPE_JPEG,
                         self.bitmap1.dataType())

    def test_setDataType(self):
        self.bitmap1.setDataType(_openraw.DataType.OR_DATA_TYPE_COMPRESSED_CFA)
        self.assertEqual(_openraw.DataType.OR_DATA_TYPE_COMPRESSED_CFA,
                         self.bitmap1.dataType())

    def test_size(self):
        self.assertEqual(944784, self.bitmap2.size())

    def test_data(self):
        # All JPEG images must start with 0xffd8 and end with 0xffd9.
        self.assert_(self.bitmap1.data().startswith('\xff\xd8'))
        self.assert_(self.bitmap1.data().endswith('\xff\xd9'))

    def test_setData(self):
        data = "".join([chr(x) for x in range(256)]) * 5

        self.assertEqual(_openraw.or_error.OR_ERROR_NONE,
                         self.bitmap1.setData(data))
        self.assertEqual(data, self.bitmap1.data())

    def test_x(self):
        self.assertEqual(160, self.bitmap1.x())

    def test_y(self):
        self.assertEqual(120, self.bitmap1.y())

    def test_bpc(self):
        self.assertEqual(8, self.bitmap1.bpc())

    def test_setBpc(self):
        self.bitmap1.setBpc(21)
        self.assertEqual(21, self.bitmap1.bpc())

    def test_setDimensions(self):
        self.bitmap1.setDimensions(50, 60)
        self.assertEqual(50, self.bitmap1.x())
        self.assertEqual(60, self.bitmap1.y())

if __name__ == '__main__':
    unittest.main()
