/*
 * Copyright (C) 2008 Brian Quinlan
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

#include <string.h>

#include "pythonrawfile.h"

PythonRawFile* PythonRawFile::newRawFile(
        const char* filename,
        OpenRaw::RawFile::Type typeHint) {

    OpenRaw::RawFile *rawfile = OpenRaw::RawFile::newRawFile(filename,
                                                             typeHint);

    if (rawfile == NULL)
        return NULL;
    else
        return new PythonRawFile(rawfile);
}

PythonRawFile* PythonRawFile::newRawFileFromMemory(
        std::string data,
        OpenRaw::RawFile::Type typeHint) {
    void * dataCopy = malloc(data.size());
    memcpy(dataCopy, data.data(), data.size());

    OpenRaw::RawFile* rawfile = OpenRaw::RawFile::newRawFileFromMemory(
            (const uint8_t *) dataCopy, data.size(), typeHint);

    if (rawfile == NULL)
        return NULL;
    else
        return new PythonMemoryRawFile(dataCopy, rawfile);
}
