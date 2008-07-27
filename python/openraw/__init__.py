# Copyright (C) 2008 Brian Quinlan
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

import _openraw

from _openraw import Type
from _openraw import or_error
from _openraw import DataType
from _openraw import or_options

DATATYPE_TO_MIMETYPE = {DataType.OR_DATA_TYPE_JPEG: 'image/jpeg',
                        DataType.OR_DATA_TYPE_TIFF: 'image/tiff',
                        DataType.OR_DATA_TYPE_PNG: 'image/png'}

class Error(Exception):
    def __init__(self, or_error_code=None):
        self.or_error_code = or_error_code

class RawFile(object):
    """Represents a raw file."""

    def __init__(self, rawfile):
        """Constructor for openraw.RawFile.

        NOTE: this constructor should be considered private. To create new
        RawFile instances, use one of the class' static factory methods.

        Args:
            rawfile: An _openraw.RawFile instance.
        """
        self._rawfile = rawfile

    @classmethod
    def from_path(cls, path, raw_type_hint=Type.OR_RAWFILE_TYPE_UNKNOWN):
        """Returns a RawFile instance for the file at the given path.
        
        Args:
            path: A string representing the path to a raw file.
            raw_type_hint: Aconstant from the openraw.Type enumeration. If not
                specified then the type of RAW file being openned with be
                guessed.
        """

        rawfile = _openraw.RawFile.newRawFile(path, raw_type_hint)
        if rawfile is None:
            raise Error()
        return RawFile(rawfile)

    @classmethod
    def from_string(cls, data, raw_type_hint=Type.OR_RAWFILE_TYPE_UNKNOWN):
        """Returns a RawFile instance given a string containing a raw file.
        
        Args:
            data: A string containing data for a raw file.
            raw_type_hint: Aconstant from the openraw.Type enumeration. If not
                specified then the type of RAW file being openned with be
                guessed.
        """

        rawfile = _openraw.RawFile.newRawFileFromMemory(data, raw_type_hint)
        if rawfile is None:
            raise Error()
        return RawFile(rawfile)

    def get_type(self):
        """Returns the raw file type as a openraw.Type enumeration constant."""
        return self._rawfile.type()

    def get_orientation(self):
        # XXX: What are the expected return values of this method?
        return self._rawfile.getOrientation()

    def get_thumbnail_widths(self):
        """Returns the widths of all the contained thumbnails."""
        return self._rawfile.listThumbnailSizes()

    def get_thumbnails(self):
        """Returns the list of contained openraw.Thumbnail instances."""
        return [self.get_thumbnail(width) for width
                in self.get_thumbnail_widths()]

    def get_thumbnail(self, requested_width):
        """Returns the openraw.Thumbnail instance with the best width match.
        
        Args:
            requested_width: An int representing the width of the desired
                thumbnail. If no thumbnail with the given width exists then the
                closest match is returned.
        """
        thumbnail = _openraw.Thumbnail()
        error = self._rawfile.getThumbnail(requested_width, thumbnail)
        if error != or_error.OR_ERROR_NONE:
            raise Error(error)
        else:
            return Thumbnail(thumbnail)

    def get_raw_data(self, options=or_options.OR_OPTIONS_NONE):
        """Returns the RawFile's data as a openraw.RawData instance."""
        rawdata = _openraw.RawData()
        error = self._rawfile.getRawData(rawdata, options)
        if error != or_error.OR_ERROR_NONE:
            raise Error(error)
        else:
            return RawData(rawdata)

class Bitmap(object):
    def __init__(self, bitmap):
        """Constructor for openraw.Bitmap.

        NOTE: this constructor should be considered private.

        Args:
            bitmap: An _openraw.Bitmap instance.
        """
        self._bitmap = bitmap

    def __repr__(self):
        return '<Bitmap data_type=%s size=%d width=%d height=%d>' % (
                self.get_data_type(),
                self.get_size(),
                self.get_width(),
                self.get_height())
    __str__ = __repr__

    def get_data_type(self):
        """Returns the bitmap type as a openraw.DataType constant."""
        return self._bitmap.dataType()

    def get_size(self):
        """Returns the size of the bitmap in bytes."""
        return self._bitmap.size()

    def get_data(self):
        """Returns the raw bitmap data as a string."""
        return self._bitmap.data()

    def get_width(self):
        """Returns the width of the bitmap in pixels."""
        return self._bitmap.x()

    def get_height(self):
        """Returns the height of the bitmap in pixels."""
        return self._bitmap.y()

    def get_bits_per_channel(self):
        """Returns the number of bits per channel in the bitmap e.g. 8."""
        return self._bitmap.bpc()

class Thumbnail(Bitmap):
    pass

class RawData(Bitmap):
    def __init__(self, rawdata):
        self._rawdata = rawdata

        Bitmap.__init__(self, rawdata)
