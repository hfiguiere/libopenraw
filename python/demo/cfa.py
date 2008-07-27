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

import os
import webbrowser

import openraw

def extract_raw_image(rawfile_path,
                      destination_path,
                      open_in_browser=False):

    raw_file = openraw.RawFile.from_path(rawfile_path)
    raw_data = raw_file.get_raw_data(openraw.or_options.OR_OPTIONS_NONE)

    extract_base_name, _ = os.path.splitext(os.path.basename(rawfile_path))

    image_path = os.path.abspath(os.path.join(destination_path,
                                              extract_base_name + '.pgm'))
    f = open(image_path, 'wb')
    f.write('P5\n')
    f.write('%d %d\n' % (raw_data.get_width(), raw_data.get_height()))

    colors_per_channel = 2 ** raw_data.get_bits_per_channel() - 1
    f.write('%d\n' % colors_per_channel)
    f.write(raw_data.get_data())

    f.close()

    if open_in_browser:
        webbrowser.open_new(image_path)

def main():
    from optparse import OptionParser

    usage = 'usage: %prog [options] rawfile1 rawfile2 ... - extracts raw data'

    parser = OptionParser(usage)
    parser.add_option('-b',
                      '--browser',
                      action='store_true',
                      dest='browser',
                      default=False,
                      help='opens the extracted raw image in a browser window')

    parser.add_option('-d',
                      '--destination',
                      type='string',
                      dest='destination',
                      default='.',
                      help='the path to which the raw image will be extracted')

    options, args = parser.parse_args()

    if not args:
        parser.error('at least one RAW file must be specified')

    for raw_file in args:
        extract_raw_image(rawfile_path=raw_file,
                          destination_path=options.destination,
                          open_in_browser=options.browser)


if __name__ == '__main__':
    main()
