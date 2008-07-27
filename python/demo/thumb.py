#!/usr/bin/env python

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

import cStringIO
import os
import mimetypes
import sys
import webbrowser

import openraw

def write_rgb8pixmap_to_stream(bitmap, stream):
    stream.write('P6\n')
    stream.write('%d %d\n' % (bitmap.get_width(), bitmap.get_height()))
    stream.write('255\n')
    stream.write(bitmap.get_data())

def extract_thumbnails(rawfile_path, destination_path, open_in_browser=False):
    raw_file = openraw.RawFile.from_path(rawfile_path)
    thumbnail_base_name, _ = os.path.splitext(os.path.basename(rawfile_path))

    for thumbnail in raw_file.get_thumbnails():
        if (thumbnail.get_data_type() ==
                openraw.DataType.OR_DATA_TYPE_PIXMAP_8RGB):
            stream = cStringIO.StringIO()
            write_rgb8pixmap_to_stream(thumbnail, stream)
            extension = '.ppm'
            data = stream.getvalue()
        else:
            try:
                mime_type = openraw.DATATYPE_TO_MIMETYPE[
                        thumbnail.get_data_type()]
            except KeyError:
                print >> sys.stderr, (
                    'Cannot extract thumbnail of type "%s" from %r (%dx%d) ' % (
                            thumbnail.get_data_type(),
                            rawfile_path,
                            thumbnail.get_width(),
                            thumbnail.get_height()))
                continue
            else:
                extension = mimetypes.guess_extension(mime_type, strict=False)
                data= thumbnail.get_data()

        thumbnail_name = '%s-%dx%d%s' % (thumbnail_base_name,
                                         thumbnail.get_width(),
                                         thumbnail.get_height(),
                                         extension)
        thumbnail_path = os.path.abspath(
                os.path.join(destination_path, thumbnail_name))

        f = open(thumbnail_path, 'wb')
        f.write(data)
        f.close()

        if open_in_browser:
            webbrowser.open_new_tab(thumbnail_path)

def main():
    from optparse import OptionParser

    usage = 'usage: %prog [options] rawfile1 rawfile2 ... - extracts thumbnails'

    parser = OptionParser(usage)
    parser.add_option('-b',
                      '--browser',
                      action='store_true',
                      dest='browser',
                      default=False,
                      help='opens the extracted thumbnails in a browser window')

    parser.add_option('-d',
                      '--destination',
                      type='string',
                      dest='destination',
                      default='.',
                      help='the path to which the thumbnails will be extracted')

    options, args = parser.parse_args()

    if not args:
        parser.error('at least one RAW file must be specified')

    for raw_file in args:
        extract_thumbnails(rawfile_path=raw_file,
                           destination_path=options.destination,
                           open_in_browser=options.browser)

if __name__ == '__main__':
    main()
