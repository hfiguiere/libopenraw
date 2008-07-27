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

#ifndef __OPENRAW_PYTONRAWFILE_H__
#define __OPENRAW_PYTONRAWFILE_H__

#include "libopenraw++/rawfile.h"
#include "libopenraw++/thumbnail.h"

/**
 * A facade that provides a more convenient interface to the OpenRaw::RawFile
 * class.
 */
class PythonRawFile {
public:
    PythonRawFile(OpenRaw::RawFile* _rawfile) {
        rawfile = _rawfile;
    }

    virtual ~PythonRawFile() {
        delete this->rawfile;
    }

    virtual OpenRaw::RawFile::Type type() const {
        return rawfile->type();
    }

    virtual OpenRaw::RawFile::TypeId typeId() {
        return rawfile->typeId();
    }

    virtual const std::vector<uint32_t>& listThumbnailSizes(void) {
        return rawfile->listThumbnailSizes();
    }

    virtual or_error getThumbnail(uint32_t size,
                                  OpenRaw::Thumbnail& thumbnail) {
        return rawfile->getThumbnail(size, thumbnail);
    }

    virtual ::or_error getRawData(OpenRaw::RawData& rawdata, uint32_t options) {
        return rawfile->getRawData(rawdata, options);
    }

    virtual int32_t getOrientation() {
        return rawfile->getOrientation();
    }

    static PythonRawFile* newRawFile(const char* filename,
                                     OpenRaw::RawFile::Type typeHint);

    static PythonRawFile* newRawFileFromMemory(std::string data,
                                               OpenRaw::RawFile::Type typeHint);

protected:
    OpenRaw::RawFile* rawfile;

    PythonRawFile(const PythonRawFile&);
    PythonRawFile & operator=(const PythonRawFile &);
};

/*
 * A class that takes overship of the image data used by the managed
 * Open::RawFile instance.
 *
 * Allows for safe and leak-free memory management.
 */
class PythonMemoryRawFile : public PythonRawFile {
public:
    PythonMemoryRawFile(void* _data, OpenRaw::RawFile* rawfile) :
            PythonRawFile(rawfile) {
        data = _data;
    }

    virtual ~PythonMemoryRawFile() {
        free(data);
    }

protected:
    void *data;
};

#endif
