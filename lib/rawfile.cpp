/*
 * libopenraw - rawfile.cpp
 *
 * Copyright (C) 2008 Novell, Inc.
 * Copyright (C) 2006-2016 Hubert Figuiere
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


#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "trace.hpp"

#include <libopenraw/metadata.h>
#include <libopenraw/cameraids.h>
#include <libopenraw/consts.h>
#include <libopenraw/debug.h>

#include "rawfile.hpp"
#include "rawdata.hpp"
#include "thumbnail.hpp"
#include "metavalue.hpp"

#include "io/stream.hpp"
#include "io/file.hpp"
#include "io/memstream.hpp"
#include "rawcontainer.hpp"
#include "tiffepfile.hpp"
#include "cr2file.hpp"
#include "neffile.hpp"
#include "orffile.hpp"
#include "arwfile.hpp"
#include "peffile.hpp"
#include "crwfile.hpp"
#include "erffile.hpp"
#include "dngfile.hpp"
#include "mrwfile.hpp"
#include "rw2file.hpp"
#include "raffile.hpp"
#include "exception.hpp"
#include "rawfile_private.hpp"

#include "rawfilefactory.hpp"

using std::string;
using namespace Debug;

namespace OpenRaw {

class BitmapData;

using Internals::RawFileFactory;

void init(void)
{
    using namespace std::placeholders;

    static RawFileFactory fctcr2(OR_RAWFILE_TYPE_CR2,
                                 std::bind(&Internals::Cr2File::factory, _1),
                                 "cr2");
    static RawFileFactory fctnef(OR_RAWFILE_TYPE_NEF,
                                 std::bind(&Internals::NefFile::factory, _1),
                                 "nef");
    static RawFileFactory fctnrw(OR_RAWFILE_TYPE_NRW,
                                 std::bind(&Internals::NefFile::factory, _1),
                                 "nrw");
    static RawFileFactory fctarw(OR_RAWFILE_TYPE_ARW,
                                 std::bind(&Internals::ArwFile::factory, _1),
                                 "arw");
    static RawFileFactory fctorf(OR_RAWFILE_TYPE_ORF,
                                 std::bind(&Internals::OrfFile::factory, _1),
                                 "orf");
    static RawFileFactory fctdng(OR_RAWFILE_TYPE_DNG,
                                 std::bind(&Internals::DngFile::factory, _1),
                                 "dng");
    static RawFileFactory fctpef(OR_RAWFILE_TYPE_PEF,
                                 std::bind(&Internals::PEFFile::factory, _1),
                                 "pef");
    static RawFileFactory fctcrw(OR_RAWFILE_TYPE_CRW,
                                 std::bind(&Internals::CRWFile::factory, _1),
                                 "crw");
    static RawFileFactory fcterf(OR_RAWFILE_TYPE_ERF,
                                 std::bind(&Internals::ERFFile::factory, _1),
                                 "erf");
    static RawFileFactory fctmrw(OR_RAWFILE_TYPE_MRW,
                                 std::bind(&Internals::MRWFile::factory, _1),
                                 "mrw");
    static RawFileFactory fctraw(OR_RAWFILE_TYPE_RW2,
                                 std::bind(&Internals::Rw2File::factory, _1),
                                 "raw");
    static RawFileFactory fctrw2(OR_RAWFILE_TYPE_RW2,
                                 std::bind(&Internals::Rw2File::factory, _1),
                                 "rw2");
    static RawFileFactory fctraf(OR_RAWFILE_TYPE_RAF,
                                 std::bind(&Internals::RafFile::factory, _1),
                                 "raf");
}

class RawFile::Private
{
public:
    Private(Type t)
        : m_type(t),
          m_type_id(OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NONE, OR_TYPEID_UNKNOWN)),
          m_sizes(),
          m_cam_ids(NULL),
          m_matrices(NULL)
        {
        }
    ~Private()
        {
            for(auto value : m_metadata)
            {
                if(value.second) {
                    delete value.second;
                }
            }
        }
    /** the real type of the raw file */
    Type m_type;
    /** the raw file type id */
    TypeId m_type_id;
    /** list of thumbnail sizes */
    std::vector<uint32_t> m_sizes;
    Internals::ThumbLocations    m_thumbLocations;
    std::map<int32_t, MetaValue*> m_metadata;
    const camera_ids_t *m_cam_ids;
    const Internals::BuiltinColourMatrix* m_matrices;
};


const char **RawFile::fileExtensions()
{
    init();

    return RawFileFactory::fileExtensions();
}


RawFile *RawFile::newRawFile(const char*_filename, RawFile::Type _typeHint)
{
    init();

    Type type;
    if (_typeHint == OR_RAWFILE_TYPE_UNKNOWN) {
        type = identify(_filename);
    }
    else {
        type = _typeHint;
    }
    LOGDBG1("factory size %ld\n", RawFileFactory::table().size());
    auto iter = RawFileFactory::table().find(type);
    if (iter == RawFileFactory::table().end()) {
        LOGWARN("factory not found\n");
        return NULL;
    }
    if (iter->second == NULL) {
        LOGWARN("factory is NULL\n");
        return NULL;
    }
    IO::Stream::Ptr f(new IO::File(_filename));
    return iter->second(f);
}

RawFile *RawFile::newRawFileFromMemory(const uint8_t *buffer,
                                       uint32_t len,
                                       RawFile::Type _typeHint)
{
    init();
    Type type;
    if (_typeHint == OR_RAWFILE_TYPE_UNKNOWN) {
        ::or_error err = identifyBuffer(buffer, len, type);
        if(err != OR_ERROR_NONE) {
            LOGERR("error identifying buffer\n");
            return NULL;
        }
    }
    else {
        type = _typeHint;
    }
    auto iter = RawFileFactory::table().find(type);
    if (iter == RawFileFactory::table().end()) {
        LOGWARN("factory not found\n");
        return NULL;
    }
    if (iter->second == nullptr) {
        LOGWARN("factory is NULL\n");
        return NULL;
    }
    IO::Stream::Ptr f(new IO::MemStream((void*)buffer, len));
    return iter->second(f);
}


RawFile::Type RawFile::identify(const char*_filename)
{
    const char *e = ::strrchr(_filename, '.');
    if (e == NULL) {
        LOGDBG1("Extension not found\n");
        return OR_RAWFILE_TYPE_UNKNOWN;
    }
    std::string extension(e + 1);
    if (extension.length() > 3) {
        return OR_RAWFILE_TYPE_UNKNOWN;
    }

    boost::to_lower(extension);

    RawFileFactory::Extensions & extensions = RawFileFactory::extensions();
    auto iter = extensions.find(extension);
    if (iter == extensions.end())
    {
        return OR_RAWFILE_TYPE_UNKNOWN;
    }
    return iter->second;
}

::or_error RawFile::identifyBuffer(const uint8_t* buff, size_t len,
                                   RawFile::Type &_type)
{
    _type = OR_RAWFILE_TYPE_UNKNOWN;
    if(len <= 4) {
        return OR_ERROR_BUF_TOO_SMALL;
    }
    if(memcmp(buff, "\0MRM", 4) == 0) {
        _type = OR_RAWFILE_TYPE_MRW;
        return OR_ERROR_NONE;
    }
    if(memcmp(buff, "II\x1a\0\0\0HEAPCCDR", 14) == 0) {
        _type = OR_RAWFILE_TYPE_CRW;
        return OR_ERROR_NONE;
    }
    if(memcmp(buff, "IIRO", 4) == 0) {
        _type = OR_RAWFILE_TYPE_ORF;
        return OR_ERROR_NONE;
    }
    if(memcmp(buff, "IIU\0", 4) == 0) {
        _type = OR_RAWFILE_TYPE_RW2;
        return OR_ERROR_NONE;
    }
    if(memcmp(buff, RAF_MAGIC, RAF_MAGIC_LEN) == 0) {
        _type = OR_RAWFILE_TYPE_RAF;
        return OR_ERROR_NONE;
    }
    if((memcmp(buff, "II\x2a\0", 4) == 0)
       || (memcmp(buff, "MM\0\x2a", 4) == 0)) {
        // TIFF based format
        if(len >=12 ) {
            if(memcmp(buff + 8, "CR\x2", 3) == 0) {
                _type = OR_RAWFILE_TYPE_CR2;
                return OR_ERROR_NONE;
            }
        }
        if(len >= 8) {
            IO::Stream::Ptr s(new IO::MemStream((void*)buff, len));
            std::unique_ptr<Internals::TiffEpFile> f(new Internals::TiffEpFile(s, OR_RAWFILE_TYPE_TIFF));

            // Take into account DNG by checking the DNGVersion tag
            const MetaValue *dng_version = f->getMetaValue(META_NS_TIFF | TIFF_TAG_DNG_VERSION);
            if(dng_version) {
                LOGDBG1("found DNG versions\n");
                _type = OR_RAWFILE_TYPE_DNG;
                return OR_ERROR_NONE;
            }

            const MetaValue *makev = f->getMetaValue(META_NS_TIFF | EXIF_TAG_MAKE);
            if(makev){
                std::string makes = makev->getString(0);
                if(makes == "NIKON CORPORATION") {
                    _type = OR_RAWFILE_TYPE_NEF;
                }
                else if(makes == "SEIKO EPSON CORP."){
                    _type = OR_RAWFILE_TYPE_ERF;
                }
                else if(makes == "PENTAX Corporation ") {
                    _type = OR_RAWFILE_TYPE_PEF;
                }
                else if(makes == "SONY           ") {
                    _type = OR_RAWFILE_TYPE_ARW;
                }
                else if(makes == "Canon") {
                    _type = OR_RAWFILE_TYPE_CR2;
                }
            }
        }

    }
    return OR_ERROR_NONE;
}

RawFile::RawFile(RawFile::Type _type)
    : d(new Private(_type))
{
}


RawFile::~RawFile()
{
    delete d;
}


RawFile::Type RawFile::type() const
{
    return d->m_type;
}

RawFile::TypeId RawFile::typeId()
{
    if(d->m_type_id == 0) {
        _identifyId();
    }
    return d->m_type_id;
}

RawFile::TypeId RawFile::_typeId() const
{
    return d->m_type_id;
}

void RawFile::_setTypeId(RawFile::TypeId _type_id)
{
    d->m_type_id = _type_id;
}

const std::vector<uint32_t> & RawFile::listThumbnailSizes(void)
{
    if (d->m_sizes.empty()) {
        LOGDBG1("_enumThumbnailSizes init\n");
        ::or_error ret = _enumThumbnailSizes(d->m_sizes);
        if (ret != OR_ERROR_NONE) {
            LOGDBG1("_enumThumbnailSizes failed\n");
        }
    }
    return d->m_sizes;
}


::or_error RawFile::getThumbnail(uint32_t tsize, Thumbnail & thumbnail)
{
    ::or_error ret = OR_ERROR_NOT_FOUND;
    uint32_t smallest_bigger = 0xffffffff;
    uint32_t biggest_smaller = 0;
    uint32_t found_size = 0;

    LOGDBG1("requested size %u\n", tsize);

    auto sizes(listThumbnailSizes());

    for (auto s : sizes) {
        LOGDBG1("current iter is %u\n", s);
        if (s < tsize) {
            if (s > biggest_smaller) {
                biggest_smaller = s;
            }
        }
        else if(s > tsize) {
            if(s < smallest_bigger) {
                smallest_bigger = s;
            }
        }
        else { // s == tsize
            found_size = tsize;
            break;
        }
    }

    if (found_size == 0) {
        found_size = (smallest_bigger != 0xffffffff ?
                      smallest_bigger : biggest_smaller);
    }

    if (found_size != 0) {
        LOGDBG1("size %u found\n", found_size);
        ret = _getThumbnail(found_size, thumbnail);
    }
    else {
        // no size found, let's fail gracefuly
        LOGDBG1("no size found\n");
        ret = OR_ERROR_NOT_FOUND;
    }

    return ret;
}

/**
 * Internal implementation of getThumbnail. The size must match.
 */
::or_error RawFile::_getThumbnail(uint32_t size, Thumbnail & thumbnail)
{
  ::or_error ret = OR_ERROR_NOT_FOUND;
  auto iter = d->m_thumbLocations.find(size);
  if(iter != d->m_thumbLocations.end())
  {
    const Internals::ThumbDesc & desc = iter->second;
    thumbnail.setDataType(desc.type);
    uint32_t byte_length= desc.length; /**< of the buffer */
    uint32_t offset = desc.offset;

    LOGDBG1("Thumbnail at %u of %u bytes.\n", offset, byte_length);

    if (byte_length != 0) {
      void *p = thumbnail.allocData(byte_length);
      size_t real_size = getContainer()->fetchData(p, offset,
                                                byte_length);
      if (real_size < byte_length) {
        LOGWARN("Size mismatch for data: got %lu expected %u ignoring.\n",
                real_size, byte_length);
      }

      thumbnail.setDimensions(desc.x, desc.y);
      ret = OR_ERROR_NONE;
    }
  }

  return ret;
}

void RawFile::_addThumbnail(uint32_t size, const Internals::ThumbDesc& desc)
{
    d->m_thumbLocations[size] = desc;
}

::or_error RawFile::getRawData(RawData & rawdata, uint32_t options)
{
    LOGDBG1("getRawData()\n");
    ::or_error ret = _getRawData(rawdata, options);
    if (ret != OR_ERROR_NONE) {
        return ret;
    }

    // if the colour matrix isn't copied already, do it now.
    uint32_t matrix_size = 0;
    if (!rawdata.getColourMatrix1(matrix_size) || !matrix_size) {
        matrix_size = colourMatrixSize();
        double *matrix = new double[matrix_size];
        if (getColourMatrix1(matrix, matrix_size) == OR_ERROR_NONE) {
            rawdata.setColourMatrix1(matrix, matrix_size);
        }
        delete [] matrix;
    }

    return ret;
}

::or_error RawFile::getRenderedImage(BitmapData & bitmapdata, uint32_t options)
{
    RawData rawdata;
    LOGDBG1("options are %u\n", options);
    ::or_error ret = getRawData(rawdata, options);
    if(ret == OR_ERROR_NONE) {
        ret = rawdata.getRenderedImage(bitmapdata, options);
    }
    return ret;
}


int32_t RawFile::getOrientation()
{
    int32_t idx = 0;
    const MetaValue * value = getMetaValue(META_NS_TIFF
                                           | EXIF_TAG_ORIENTATION);
    if(value == NULL) {
        return 0;
    }
    try {
        idx = value->getInteger(0);
    }
    catch(const Internals::BadTypeException & e)	{
        LOGDBG1("wrong type - %s\n", e.what());
    }
    return idx;
}

uint32_t RawFile::colourMatrixSize()
{
    return 9;
}

::or_error RawFile::getColourMatrix1(double* matrix, uint32_t & size)
{
    return _getColourMatrix(1, matrix, size);
}

::or_error RawFile::getColourMatrix2(double* matrix, uint32_t & size)
{
    return _getColourMatrix(2, matrix, size);
}

::or_error RawFile::_getColourMatrix(uint32_t index, double* matrix, uint32_t & size)
{
    int32_t meta_index = 0;
    switch(index) {
    case 1:
        meta_index = META_NS_TIFF | DNG_TAG_COLORMATRIX1;
        break;
    case 2:
        meta_index = META_NS_TIFF | DNG_TAG_COLORMATRIX2;
        break;
    default:
        size = 0;
        return OR_ERROR_INVALID_PARAM;
    }
    const MetaValue* meta = getMetaValue(meta_index);

    if(!meta) {
        if (index != 1) {
            size = 0;
            return OR_ERROR_INVALID_PARAM;
        }
        return _getBuiltinColourMatrix(d->m_matrices, typeId(), matrix, size);
    }
    uint32_t count = meta->getCount();
    if(size < count) {
        // return the expected size
        size = count;
        return OR_ERROR_BUF_TOO_SMALL;
    }

    for(uint32_t i = 0; i < count; i++) {
        matrix[i] = meta->getDouble(i);
    }
    size = count;

    return OR_ERROR_NONE;
}

ExifLightsourceValue RawFile::getCalibrationIlluminant1()
{
    return _getCalibrationIlluminant(1);
}

ExifLightsourceValue RawFile::getCalibrationIlluminant2()
{
    return _getCalibrationIlluminant(2);
}

ExifLightsourceValue RawFile::_getCalibrationIlluminant(uint16_t index)
{
    int32_t meta_index = 0;
    switch(index) {
    case 1:
        meta_index = META_NS_TIFF | DNG_TAG_CALIBRATION_ILLUMINANT1;
        break;
    case 2:
        meta_index = META_NS_TIFF | DNG_TAG_CALIBRATION_ILLUMINANT2;
        break;
    default:
        return EV_LIGHTSOURCE_UNKNOWN;
    }
    const MetaValue* meta = getMetaValue(meta_index);

    if(!meta) {
        return (index == 1) ? EV_LIGHTSOURCE_D65 : EV_LIGHTSOURCE_UNKNOWN;
    }
    return (ExifLightsourceValue)meta->getInteger(0);
}

const MetaValue *RawFile::getMetaValue(int32_t meta_index)
{
    MetaValue *val = NULL;
    auto iter = d->m_metadata.find(meta_index);
    if(iter == d->m_metadata.end()) {
        val = _getMetaValue(meta_index);
        if(val != NULL) {
            d->m_metadata[meta_index] = val;
        }
    }
    else {
        val = iter->second;
    }
    return val;
}


const RawFile::camera_ids_t*
RawFile::_lookupCameraId(const camera_ids_t * map, const std::string& value)
{
    const camera_ids_t * p = map;
    if(!p) {
        return NULL;
    }
    while(p->model) {
        if(value == p->model) {
            return p;
        }
        p++;
    }
    return NULL;
}

RawFile::TypeId RawFile::_typeIdFromModel(const std::string & make,
                                          const std::string & model)
{
    const camera_ids_t * p = _lookupCameraId(d->m_cam_ids, model);
    if (!p) {
        return _typeIdFromMake(make);
    }
    return p->type_id;
}

const RawFile::camera_ids_t RawFile::s_make[] = {
    { "Canon", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, 0) },
    { "NIKON CORPORATION", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 0) },
    { "LEICA CAMERA AG        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA, 0) },
    { "Leica Camera AG", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA, 0) },
    { "Panasonic", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC, 0) },
    // Hardcoded
    { "Minolta", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_MINOLTA, 0) },
    { "FujiFilm", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM, 0) },
    { NULL, 0 }
};


RawFile::TypeId
RawFile::_typeIdFromMake(const std::string& make)
{
    const camera_ids_t * p = _lookupCameraId(s_make, make);
    if (!p) {
        return 0;
    }
    return p->type_id;
}

void RawFile::_setIdMap(const camera_ids_t *map)
{
    d->m_cam_ids = map;
}

const Internals::BuiltinColourMatrix*
RawFile::_getMatrices() const
{
    return d->m_matrices;
}

void RawFile::_setMatrices(const Internals::BuiltinColourMatrix* matrices)
{
    d->m_matrices = matrices;
}

::or_error
RawFile::_getBuiltinLevels(const Internals::BuiltinColourMatrix* m,
                           TypeId type_id,
                           uint16_t & black, uint16_t & white)
{
    if(!m) {
        return OR_ERROR_NOT_FOUND;
    }
    while(m->camera) {
        if(m->camera == type_id) {
            black = m->black;
            white = m->white;
            return OR_ERROR_NONE;
        }
        ++m;
    }
    return OR_ERROR_NOT_FOUND;
}

::or_error
RawFile::_getBuiltinColourMatrix(const Internals::BuiltinColourMatrix* m, 
                                 TypeId type_id,
                                 double* matrix,
                                 uint32_t & size)
{
    if(!m) {
        return OR_ERROR_NOT_FOUND;
    }
    if(size < 9) {
        return OR_ERROR_BUF_TOO_SMALL;
    }

    while(m->camera) {
        if(m->camera == type_id) {
            for(int i = 0; i < 9; i++) {
                matrix[i] = static_cast<double>(m->matrix[i]) / 10000.0;
            }
            size = 9;
            return OR_ERROR_NONE;
        }
        ++m;
    }
    size = 0;
    return OR_ERROR_NOT_FOUND;
}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/

