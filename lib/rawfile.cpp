/*
 * libopenraw - rawfile.cpp
 *
 * Copyright (C) 2008 Novell, Inc.
 * Copyright (C) 2006-2008, 2010 Hubert Figuiere
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


#include <cstring>
#include <cassert>
#include <map>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/checked_delete.hpp>

#include "trace.h"

#include <libopenraw/metadata.h>
#include <libopenraw++/rawfile.h>
#include <libopenraw++/rawdata.h>
#include <libopenraw++/thumbnail.h>

#include "io/file.h"
#include "io/memstream.h"
#include "cr2file.h"
#include "neffile.h"
#include "orffile.h"
#include "arwfile.h"
#include "peffile.h"
#include "crwfile.h"
#include "erffile.h"
#include "dngfile.h"
#include "mrwfile.h"
#include "metavalue.h"
#include "exception.h"
#include "demosaic.h"

#include "rawfilefactory.h"

using std::string;
using namespace Debug;

namespace OpenRaw {

using Internals::RawFileFactory;

void init(void)
{
    static RawFileFactory fctcr2(OR_RAWFILE_TYPE_CR2, 
                                 boost::bind(&Internals::Cr2File::factory, _1),
                                 "cr2");
    static RawFileFactory fctnef(OR_RAWFILE_TYPE_NEF, 
                                 boost::bind(&Internals::NEFFile::factory, _1),
                                 "nef");
    static RawFileFactory fctarw(OR_RAWFILE_TYPE_ARW, 
                                 boost::bind(&Internals::ARWFile::factory, _1),
                                 "arw");
    static RawFileFactory fctorf(OR_RAWFILE_TYPE_ORF, 
                                 boost::bind(&Internals::OrfFile::factory, _1),
                                 "orf");
    static RawFileFactory fctdng(OR_RAWFILE_TYPE_DNG, 
                                 boost::bind(&Internals::DNGFile::factory, _1),
                                 "dng");
    static RawFileFactory fctpef(OR_RAWFILE_TYPE_PEF, 
                                 boost::bind(&Internals::PEFFile::factory, _1),
                                 "pef");
    static RawFileFactory fctcrw(OR_RAWFILE_TYPE_CRW,
                                 boost::bind(&Internals::CRWFile::factory, _1),
                                 "crw");
    static RawFileFactory fcterf(OR_RAWFILE_TYPE_ERF,
                                 boost::bind(&Internals::ERFFile::factory, _1),
                                 "erf");
    static RawFileFactory fctmrw(OR_RAWFILE_TYPE_MRW,
                                 boost::bind(&Internals::MRWFile::factory, _1),
                                 "mrw");
}

class RawFile::Private 
{
public:
    Private(Type t)
        : m_type(t),
          m_type_id(OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NONE, OR_TYPEID_UNKNOWN)),
          m_sizes(),
          m_cam_ids(NULL)
        {
        }
    ~Private()
        {
            std::map<int32_t, MetaValue*>::iterator iter;
            for(iter = m_metadata.begin();
                iter != m_metadata.end(); ++iter)
            {
                if(iter->second) {
                    delete iter->second;
                }
            }
        }
    /** the real type of the raw file */
    Type m_type;
    /** the raw file type id */
    TypeId m_type_id;
    /** list of thumbnail sizes */
    std::vector<uint32_t> m_sizes;
    std::map<int32_t, MetaValue*> m_metadata;
    const camera_ids_t *m_cam_ids;
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
    Trace(DEBUG1) << "factory size " << RawFileFactory::table().size() << "\n";
    RawFileFactory::Table::iterator iter = RawFileFactory::table().find(type);
    if (iter == RawFileFactory::table().end()) {
        Trace(WARNING) << "factory not found\n";
        return NULL;
    }
    if (iter->second == NULL) {
        Trace(WARNING) << "factory is NULL\n";
        return NULL;
    }
    IO::Stream *f = new IO::File(_filename);
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
            Trace(ERROR) << "error identifying buffer\n";
            return NULL;
        }
    }
    else {
        type = _typeHint;
    }
    RawFileFactory::Table::iterator iter = RawFileFactory::table().find(type);
    if (iter == RawFileFactory::table().end()) {
        Trace(WARNING) << "factory not found\n";
        return NULL;
    }
    if (iter->second == NULL) {
        Trace(WARNING) << "factory is NULL\n";
        return NULL;
    }
    IO::Stream *f = new IO::MemStream((void*)buffer, len);
    return iter->second(f);
}


RawFile::Type RawFile::identify(const char*_filename)
{
    const char *e = ::strrchr(_filename, '.');
    if (e == NULL) {
        Trace(DEBUG1) << "Extension not found\n";
        return OR_RAWFILE_TYPE_UNKNOWN;
    }
    std::string extension(e + 1);
    if (extension.length() > 3) {
        return OR_RAWFILE_TYPE_UNKNOWN;
    }

    boost::to_lower(extension);

    RawFileFactory::Extensions & extensions = RawFileFactory::extensions();
    RawFileFactory::Extensions::iterator iter = extensions.find(extension);
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
            IO::Stream *s = new IO::MemStream((void*)buff, len);
            boost::scoped_ptr<Internals::TiffEpFile> f(new Internals::TiffEpFile(s, OR_RAWFILE_TYPE_TIFF));
            
            // Take into account DNG by checking the DNGVersion tag
            const MetaValue *dng_version = f->getMetaValue(META_NS_TIFF | TIFF_TAG_DNG_VERSION);
            if(dng_version) {
                Trace(DEBUG1) << "found DNG versions\n";
                _type = OR_RAWFILE_TYPE_DNG;
                return OR_ERROR_NONE;
            }

            const MetaValue *makev = f->getMetaValue(META_NS_TIFF | EXIF_TAG_MAKE);
            if(makev){
                std::string makes = makev->getString();
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

RawFile::RawFile(IO::Stream *, RawFile::Type _type)
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

void RawFile::_setTypeId(RawFile::TypeId _type_id)
{
    d->m_type_id = _type_id;
}

const std::vector<uint32_t> & RawFile::listThumbnailSizes(void)
{
    if (d->m_sizes.size() == 0) {
        Trace(DEBUG1) << "_enumThumbnailSizes init\n";
        bool ret = _enumThumbnailSizes(d->m_sizes);
        if (!ret) {
            Trace(DEBUG1) << "_enumThumbnailSizes failed\n";
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

    Trace(DEBUG1) << "requested size " << tsize << "\n";

    const std::vector<uint32_t> & sizes(listThumbnailSizes());

    std::vector<uint32_t>::const_iterator iter;

    for (iter = sizes.begin(); iter != sizes.end(); ++iter) {
        Trace(DEBUG1) << "current iter is " << *iter << "\n";
        if (*iter < tsize) {
            if (*iter > biggest_smaller) {
                biggest_smaller = *iter;
            }
        }
        else if(*iter > tsize) {
            if(*iter < smallest_bigger) {
                smallest_bigger = *iter;
            }
        }
        else { // *iter == tsize
            found_size = tsize;
            break;
        }
    }

    if (found_size == 0) {
        found_size = (smallest_bigger != 0xffffffff ? 
                      smallest_bigger : biggest_smaller);
    }

    if (found_size != 0) {
        Trace(DEBUG1) << "size " << found_size << " found\n";
        ret = _getThumbnail(found_size, thumbnail);
    }
    else {
        // no size found, let's fail gracefuly
        Trace(DEBUG1) << "no size found\n";
        ret = OR_ERROR_NOT_FOUND;
    }

    return ret;
}


::or_error RawFile::getRawData(RawData & rawdata, uint32_t options)
{
    Trace(DEBUG1) << "getRawData()\n";
    ::or_error ret = _getRawData(rawdata, options);
    return ret;
}	

::or_error RawFile::getRenderedImage(BitmapData & bitmapdata, uint32_t options)
{
    RawData rawdata;
    Trace(DEBUG1) << "options are " << options << "\n";
    ::or_error ret = getRawData(rawdata, options);
    if(ret == OR_ERROR_NONE) {
        if(rawdata.dataType() != OR_DATA_TYPE_CFA) {
            Trace(DEBUG1) << "wrong data type\n";
            return OR_ERROR_INVALID_FORMAT;
        }
        uint32_t x,y;
        or_cfa_pattern pattern;
        uint16_t *src;
        pattern = rawdata.cfaPattern();
        x = rawdata.x();
        y = rawdata.y();
        bitmapdata.setDimensions(x,y);
        bitmapdata.setDataType(OR_DATA_TYPE_PIXMAP_8RGB);
        uint8_t *dst = (uint8_t *)bitmapdata.allocData(sizeof(uint8_t) * 3 * x * y);
        /*
        rawdata.linearize();
        rawdata.subtractBlack();
        rawdata.rescale();
        rawdata.clip();
        */
        src = (uint16_t*)rawdata.data();

        /* figure out how the demosaic can be plugged for a different 
         * algorithm */
        bimedian_demosaic(src, x, y, pattern, dst);
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
        idx = value->getInteger();
    }
    catch(const Internals::BadTypeException & e)	{
        Trace(DEBUG1) << "wrong type - " << e.what() << "\n";
    }
    return idx;
}
	
const MetaValue *RawFile::getMetaValue(int32_t meta_index)
{
    MetaValue *val = NULL;
    std::map<int32_t, MetaValue*>::iterator iter = d->m_metadata.find(meta_index);
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


RawFile::TypeId RawFile::_typeIdFromModel(const std::string & model)
{
    const struct camera_ids_t * p = d->m_cam_ids;
    if(!p) {
        return 0;
    }
    while(p->model) {
        if(model == p->model) {
            break;
        }
        p++;
    }
    return p->type_id;
}

void RawFile::_setIdMap(const camera_ids_t *map)
{
    d->m_cam_ids = map;
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

