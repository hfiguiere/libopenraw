/*
 * libopenraw - crwfile.cpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
 * Copyright (c) 2008 Novell, Inc.
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

#include <fcntl.h>
#include <stddef.h>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <memory>
#include <map>

#include <libopenraw/debug.h>
#include <libopenraw/metadata.h>
#include <libopenraw/cameraids.h>

#include "rawdata.hpp"
#include "metavalue.hpp"
#include "mosaicinfo.hpp"
#include "rawfile.hpp"
#include "trace.hpp"
#include "io/streamclone.hpp"
#include "io/memstream.hpp"
#include "crwfile.hpp"
#include "ciffcontainer.hpp"
#include "jfifcontainer.hpp"
#include "crwdecompressor.hpp"
#include "rawfile_private.hpp"
#include "canon.hpp"
#include "ciff/ciffifd.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

using namespace CIFF;

#define OR_MAKE_CANON_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_D30), 0, 0,
	{ 9805,-2689,-1312,-5803,13064,3068,-2438,3075,8775 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_D60), 0, 0xfa0,
	{ 6188,-1341,-890,-7168,14489,2937,-2640,3228,8483 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_10D), 0, 0xfa0,
	{ 8197,-2000,-1118,-6714,14335,2592,-2536,3178,8266 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_300D), 0, 0xfa0,
	{ 8197,-2000,-1118,-6714,14335,2592,-2536,3178,8266 } },
//    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1), 0, 0,
//	{ -4778,9467,2172,4743,-1141,4344,-5146,9908,6077,-1566,11051,557 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G2), 0, 0,
	{ 9087,-2693,-1049,-6715,14382,2537,-2291,2819,7790 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G3), 0, 0,
	{ 9212,-2781,-1073,-6573,14189,2605,-2300,2844,7664 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5), 0, 0,
	{ 9757,-2872,-933,-5972,13861,2301,-1622,2328,7212 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G6), 0, 0,
	{ 9877,-3775,-871,-7613,14807,3072,-1448,1305,7485 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_PRO1), 0, 0,
	{ 10062,-3522,-999,-7643,15117,2730,-765,817,7323 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S30), 0, 0,
	{ 10566, -3652, -1129, -6552, 14662, 2006, -2197, 2581, 7670 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S40), 0, 0,
	{ 8510, -2487, -940, -6869, 14231, 2900, -2318, 2829, 9013 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S45), 0, 0,
	{ 8163, -2333, -955, -6682, 14174, 2751, -2077, 2597, 8041 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S50), 0, 0,
	{ 8882, -2571, -863, -6348, 14234, 2288, -1516, 2172, 6569 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S60), 0, 0,
	{ 8795, -2482, -797, -7804, 15403, 2573, -1422, 1996, 7082 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S70), 0, 0,
	{ 9976, -3810, -832, -7115, 14463, 2906, -901, 989, 7889 } },
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const RawFile::camera_ids_t CRWFile::s_def[] = {
    { "Canon EOS D30" , OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_D30) },
    { "Canon EOS D60" , OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_D60) },
    { "Canon EOS 10D" , OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_10D) },
    { "Canon EOS DIGITAL REBEL", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_DIGITAL_REBEL) },
    { "Canon EOS 300D DIGITAL", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_300D) },
    { "Canon PowerShot G1", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1) },
    { "Canon PowerShot G2", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G2) },
    { "Canon PowerShot G3", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G3) },
    { "Canon PowerShot G5", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5) },
    { "Canon PowerShot G6", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G6) },
// G7 is CHDK, So remove from the list from now.
//    { "Canon PowerShot G7", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7) },
    { "Canon PowerShot Pro1", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_PRO1) },
    { "Canon PowerShot Pro70", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_PRO70) },
    { "Canon PowerShot Pro90 IS", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_PRO90) },
    { "Canon PowerShot S30", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S30) },
    { "Canon PowerShot S40", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S40) },
    { "Canon PowerShot S45", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S45) },
    { "Canon PowerShot S50", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S50) },
    { "Canon PowerShot S60", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S60) },
    { "Canon PowerShot S70", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S70) },
    { 0, 0 }
};

RawFile *CRWFile::factory(const IO::Stream::Ptr &s)
{
    return new CRWFile(s);
}

CRWFile::CRWFile(const IO::Stream::Ptr &s)
    : RawFile(OR_RAWFILE_TYPE_CRW),
      m_io(s),
      m_container(new CIFFContainer(m_io)),
      m_x(0), m_y(0)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

CRWFile::~CRWFile()
{
    delete m_container;
}

::or_error CRWFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
    ::or_error err = OR_ERROR_NOT_FOUND;

    HeapRef heap = m_container->heap();
    if(!heap) {
        // this is not a CIFF file.
        return err;
    }
    const RecordEntries& records = heap->records();
    RecordEntries::const_iterator iter;
    iter = records.find(TAG_JPEGIMAGE);
    if (iter != records.end()) {
        LOGDBG2("JPEG @%u\n", iter->second.offset());
        m_x = m_y = 0;
        uint32_t offset = heap->offset() + iter->second.offset();
        IO::StreamClone::Ptr s(new IO::StreamClone(m_io, offset));
        std::unique_ptr<JfifContainer> jfif(new JfifContainer(s, 0));

        jfif->getDimensions(m_x, m_y);
        LOGDBG1("JPEG dimensions x=%d y=%d\n", m_x, m_y);
        uint32_t dim = std::max(m_x,m_y);
        _addThumbnail(dim, ThumbDesc(m_x, m_y, OR_DATA_TYPE_JPEG, offset, iter->second.length()));
        list.push_back(dim);
        err = OR_ERROR_NONE;
    }

    return err;
}

RawContainer* CRWFile::getContainer() const
{
  return m_container;
}

::or_error CRWFile::_getRawData(RawData & data, uint32_t options)
{
    ::or_error err = OR_ERROR_NOT_FOUND;

    const ImageSpec * img_spec = m_container->getImageSpec();
    uint32_t x, y;
    x = y = 0;
    if(img_spec) {
        x = img_spec->imageWidth;
        y = img_spec->imageHeight;
    }

    // locate decoder table
    HeapRef exifProps = m_container->getExifInfo();
    if (!exifProps) {
        LOGERR("Couldn't find the exif info table.\n");
        return err;
    }
    const RecordEntries& exifPropsRecs = exifProps->records();
    auto iter = exifPropsRecs.find(TAG_DECODERTABLE);
    if (iter == exifPropsRecs.end()) {
        LOGERR("Couldn't find the decoder table.\n");
        return err;
    }
    LOGDBG2("length = %d\n", iter->second.length());
    LOGDBG2("offset = %lld\n", (long long int)(exifProps->offset() + iter->second.offset()));
    auto file = m_container->file();
    file->seek(exifProps->offset() + iter->second.offset(), SEEK_SET);

    auto result = m_container->readUInt32(file, m_container->endian());
    if(result.empty()) {
        LOGERR("Couldn't find decoder table\n");
        return OR_ERROR_NOT_FOUND;
    }

    uint32_t decoderTable = result.value();
    LOGDBG2("decoder table = %u\n", decoderTable);

    // locate the CFA info
    iter = exifPropsRecs.find(TAG_SENSORINFO);
    if (iter == exifPropsRecs.end()) {
        LOGERR("Couldn't find the sensor info.\n");
        return err;
    }
    LOGDBG2("length = %u\n", iter->second.length());
    LOGDBG2("offset = %lld\n", (long long int)(exifProps->offset() + iter->second.offset()));

    // This is the SensorInfo tag
    // https://exiftool.org/TagNames/Canon.html#SensorInfo
    file->seek(exifProps->offset() + iter->second.offset(), SEEK_SET);

    std::vector<uint16_t> sensor_info;
    auto count_read = m_container->readUInt16Array(file, sensor_info, 9);
    if (count_read != 9) {
        LOGERR("SensorInfo short read %lu.\n", count_read);
        return OR_ERROR_NOT_FOUND;
    }
    LOGDBG1("read sensor info %ld\n", count_read);
    auto cfa_x = sensor_info[1];
    auto cfa_y = sensor_info[2];
    LOGDBG2("cfa, x %u, y %u\n", cfa_x, cfa_y);

    auto active_area = canon_parse_sensorinfo(sensor_info);
    if (!active_area) {
        LOGERR("SensorInfo: couldn't get active area.\n");
        return OR_ERROR_NOT_FOUND;
    }

    const RecordEntry *entry = m_container->getRawDataRecord();
    if (entry) {
        CIFF::HeapRef heap = m_container->heap();
        LOGDBG2("RAW @%lld\n", (long long int)(heap->offset() + entry->offset()));
        size_t byte_size = entry->length();
        void *buf = data.allocData(byte_size);
        size_t real_size = entry->fetchData(heap.get(), buf, byte_size);
        if (real_size != byte_size) {
            LOGWARN("wrong size\n");
        }
        data.setDimensions(x, y);
        data.setCfaPatternType(OR_CFA_PATTERN_RGGB);
        data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);

        // decompress if we need
        if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
            std::unique_ptr<IO::Stream> s(
                new IO::MemStream((const uint8_t*)data.data(), data.size()));
            s->open(); // TODO check success

            CrwDecompressor decomp(s.get(), m_container);

            decomp.setOutputDimensions(cfa_x, cfa_y);
            decomp.setDecoderTable(decoderTable);
            RawDataPtr dData = decomp.decompress();
            if (dData) {
                LOGDBG1("Out size is %dx%d\n", dData->width(), dData->height());
                dData->setCfaPatternType(data.mosaicInfo()->patternType());
                data.swap(*dData);
            }
        }
        data.setActiveArea((*active_area)[0], (*active_area)[1],
                           (*active_area)[2], (*active_area)[3]);
        err = OR_ERROR_NONE;
    }
    return err;
}

Option<uint32_t> CRWFile::getOrientation() const
{
    const ImageSpec * img_spec = m_container->getImageSpec();
    if (img_spec) {
        return static_cast<uint32_t>(img_spec->exifOrientation());
    }
    return OptionNone();
}

Option<std::string> CRWFile::getMakeOrModel(uint32_t index)
{
    if (index == EXIF_TAG_MAKE && !m_make.empty()) {
        return m_make;
    } else if (index == EXIF_TAG_MODEL && !m_model.empty()) {
        return m_model;
    }

    Option<std::string> val;
    CIFF::HeapRef heap = m_container->getCameraProps();
    if (heap) {
        auto propsRecs = heap->records();
        auto iter = propsRecs.find(TAG_RAWMAKEMODEL);
        if (iter == propsRecs.end()) {
            LOGERR("Couldn't find the image info.\n");
        } else {
            char buf[256];
            size_t sz = iter->second.length();
            if(sz > 256) {
                sz = 256;
            }
            /*size_t sz2 = */iter->second.fetchData(heap.get(),
                                                    (void*)buf, sz);
            const char *p = buf;
            while(*p) {
                p++;
            }
            m_make = std::string(buf, p - buf);
            p++;
            m_model = p;

            if (index == EXIF_TAG_MODEL) {
                val = m_model;
            } else if (index == EXIF_TAG_MAKE) {
                val = m_make;
            }
            LOGDBG1("Make %s\n", m_make.c_str());
            LOGDBG1("Model %s\n", m_model.c_str());
        }
    }
    return val;
}

MetaValue *CRWFile::_getMetaValue(int32_t meta_index)
{
    MetaValue * val = NULL;

    switch(META_INDEX_MASKOUT(meta_index)) {
    case META_NS_TIFF:
    {
        uint32_t index = META_NS_MASKOUT(meta_index);
        switch(index) {
        case EXIF_TAG_ORIENTATION:
        {
            auto orientation = getOrientation();
            if (orientation) {
                val = new MetaValue(orientation.value());
            }
            break;
        }
        case EXIF_TAG_MAKE:
        case EXIF_TAG_MODEL:
        {
            auto tag = getMakeOrModel(index);
            if (tag) {
                val = new MetaValue(tag.value());
            }
            break;
        }
        }
        break;
    }
    case META_NS_EXIF:
        break;
    default:
        LOGERR("Unknown Meta Namespace\n");
        break;
    }

    return val;
}

IfdDir::Ref CRWFile::_locateMainIfd()
{
    auto ifd = std::make_shared<CiffMainIfd>(*this, *m_container);
    ifd->load();
    return ifd;
}

IfdDir::Ref CRWFile::_locateExifIfd()
{
    auto ifd = std::make_shared<CiffExifIfd>(*this, *m_container);
    ifd->load();
    return ifd;
}

void CRWFile::_identifyId()
{
    std::string model;
    std::string make;
    try {
        MetaValue * v = _getMetaValue(META_NS_TIFF | EXIF_TAG_MODEL);
        if(v) {
            model = v->getString(0);
        }
        delete v;
        v = _getMetaValue(META_NS_TIFF | EXIF_TAG_MAKE);
        if(v) {
            make = v->getString(0);
        }
        delete v;
        _setTypeId(_typeIdFromModel(make, model));
    }
    catch(...)
    {
    }
}

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  c-basic-offset: 4
  tab-width: 4
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
