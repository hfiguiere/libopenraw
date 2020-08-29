/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ciffifd.hpp
 *
 * Copyright (C) 2020 Hubert Figuière
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

#include "ifd.hpp"
#include "ciffifd.hpp"
#include "ciffcontainer.hpp"
#include "crwfile.hpp"

namespace OpenRaw {
namespace Internals {
namespace CIFF {

CiffIfd::CiffIfd(CRWFile& ciff, RawContainer& container, IfdDirType _type)
    : IfdDir(0, container, _type)
    , m_file(ciff)
{
}

IfdEntry::Ref CiffIfd::entryForString(uint16_t id, const std::string& str) const
{
    // We include the terminating NUL.
    // std::string::c_str() returns it.
    auto entry = std::make_shared<IfdEntry>(id, IFD::EXIF_FORMAT_ASCII,
                                            str.size() + 1, 0, *this, true);
    entry->setData(reinterpret_cast<const uint8_t*>(str.c_str()), str.size() + 1);
    return entry;
}

CiffMainIfd::CiffMainIfd(CRWFile& ciff, RawContainer& container)
    : CiffIfd(ciff, container, OR_IFD_MAIN)
{
}

bool CiffMainIfd::load()
{
    auto img_spec = static_cast<CIFFContainer*>(m_file.getContainer())->getImageSpec();
    if (img_spec) {
        auto w = img_spec->imageWidth;
        auto h = img_spec->imageHeight;
        auto bpc = img_spec->componentBitDepth;

        // The data field in ifdentry is in container endian
        if (endian() == RawContainer::ENDIAN_BIG) {
            w = htobe16(w);
            h = htobe16(h);
            bpc = htobe16(bpc);
        } else {
            w = htole16(w);
            h = htole16(h);
            bpc = htole16(bpc);
        }
        auto entry = std::make_shared<IfdEntry>(EXIF_TAG_IMAGE_WIDTH, EXIF_FORMAT_SHORT,
                                                1, w, *this, true);
        m_entries[EXIF_TAG_IMAGE_WIDTH] = entry;
        entry = std::make_shared<IfdEntry>(EXIF_TAG_IMAGE_LENGTH, EXIF_FORMAT_SHORT,
                                           1, h, *this, true);
        m_entries[EXIF_TAG_IMAGE_LENGTH] = entry;
        entry = std::make_shared<IfdEntry>(EXIF_TAG_BITS_PER_SAMPLE, EXIF_FORMAT_SHORT,
                                           1, bpc, *this, true);
        m_entries[EXIF_TAG_BITS_PER_SAMPLE] = entry;
    }

    auto val = m_file.getOrientation();
    if (val) {
        auto entry = std::make_shared<IfdEntry>(EXIF_TAG_ORIENTATION, EXIF_FORMAT_SHORT,
                                                1, val.value(), *this, true);
        m_entries[EXIF_TAG_ORIENTATION] = entry;
    }
    auto val_str = m_file.getMakeOrModel(EXIF_TAG_MAKE);
    if (val_str) {
        auto entry = entryForString(EXIF_TAG_MAKE, val_str.value());
        m_entries[EXIF_TAG_MAKE] = entry;
    }
    val_str = m_file.getMakeOrModel(EXIF_TAG_MODEL);
    if (val_str) {
        auto entry = entryForString(EXIF_TAG_MODEL, val_str.value());
        m_entries[EXIF_TAG_MODEL] = entry;
    }
    return true;
}

CiffExifIfd::CiffExifIfd(CRWFile& ciff, RawContainer& container)
    : CiffIfd(ciff, container, OR_IFD_EXIF)
{
}

namespace {

typedef IfdEntry::Ref (*Converter)(const RecordEntry& e, Heap& heap, CiffIfd& ifd, uint16_t exifTag);

struct Ciff2Exif {
    uint16_t exifTag;
    or_ifd_dir_type dest;
    Option<Converter> converter;
};

// TAG_FOCALLENGTH to Exif
IfdEntry::Ref translateFocalLength(const RecordEntry& e, Heap&, CiffIfd& ifd, uint16_t exifTag)
{
    LOGASSERT(e.inRecord());
    uint32_t fl;
    uint32_t fu = 0;
    auto data = boost::get<RecordEntry::InRec>(e.data);
    if (ifd.container().endian() == RawContainer::ENDIAN_LITTLE) {
        fl = IfdTypeTrait<uint16_t>::EL(data.c_array() + 2, sizeof(uint16_t));
    } else {
        fl = IfdTypeTrait<uint16_t>::BE(data.c_array() + 2, sizeof(uint16_t));
    }

    const CIFFContainer* ciffc = dynamic_cast<const CIFFContainer*>(&ifd.container());
    auto csettings = ciffc->getCameraSettings();
    if (csettings.size() >= 26) {
        fu = csettings[25];
    }

    uint32_t r[] = { fl, fu };
    auto ifdentry = std::make_shared<IfdEntry>(exifTag, EXIF_FORMAT_RATIONAL, 1, 0, ifd, true);
    ifdentry->setData(reinterpret_cast<uint8_t*>(&r), 8);
    return ifdentry;
}

IfdEntry::Ref translateDate(const RecordEntry& e, Heap& heap, CiffIfd& ifd, uint16_t exifTag)
{
    struct tm d;
    uint32_t data[3];
    e.fetchData(&heap, &data, 12);
    time_t t = data[0];
    char date[] = "0000:00:00 00:00:00";
    auto d2 = gmtime_r(&t, &d);
    if (d2) {
        strftime(date, 20, "%Y:%m:%d %H:%M:%S", d2);
    }
    return ifd.entryForString(exifTag, date);
}

IfdEntry::Ref translateSerial(const RecordEntry& e, Heap& , CiffIfd& ifd, uint16_t exifTag)
{
    uint32_t serial_v;
    LOGASSERT(e.inRecord());
    auto data = boost::get<RecordEntry::InRec>(e.data);
    if (ifd.container().endian() == RawContainer::ENDIAN_LITTLE) {
        serial_v = IfdTypeTrait<uint32_t>::EL(data.c_array(), sizeof(uint32_t));
    } else {
        serial_v = IfdTypeTrait<uint32_t>::BE(data.c_array() + 2, sizeof(uint32_t));
    }
    char serial[10];
    snprintf(serial, 10, "%X", serial_v);
    return ifd.entryForString(exifTag, serial);
}

IfdEntry::Ref translateString(const RecordEntry& e, Heap& heap, CiffIfd& ifd, uint16_t exifTag)
{
    std::string val_str = e.getString(heap);
    return ifd.entryForString(exifTag, val_str);
}

IfdEntry::Ref translateMakeModel(const RecordEntry& e, Heap& heap, CiffIfd& ifd, uint16_t exifTag)
{
    return ifd.entryForString(exifTag, e.getString(heap));
}

static const std::multimap<uint16_t, Ciff2Exif> ciff_exif_map = {
    { TAG_FOCALLENGTH, { EXIF_TAG_FOCAL_LENGTH, OR_IFD_EXIF, &translateFocalLength } },
    { TAG_FILEDESCRIPTION, { EXIF_TAG_IMAGE_DESCRIPTION, OR_IFD_MAIN, OptionNone() } },
    { TAG_ORIGINALFILENAME, { EXIF_TAG_DOCUMENT_NAME, OR_IFD_MAIN, OptionNone() } },
    { TAG_TARGETDISTANCESETTING, { EXIF_TAG_SUBJECT_DISTANCE, OR_IFD_EXIF, OptionNone() } },
    { TAG_RAWMAKEMODEL, { EXIF_TAG_MAKE, OR_IFD_MAIN, &translateMakeModel } },
    { TAG_RAWMAKEMODEL, { EXIF_TAG_MODEL, OR_IFD_MAIN, &translateMakeModel } },
    { TAG_OWNERNAME, { EXIF_TAG_CAMERA_OWNER_NAME, OR_IFD_EXIF, &translateString } },
    { TAG_SERIALNUMBER, { EXIF_TAG_BODY_SERIAL_NUMBER, OR_IFD_EXIF, &translateSerial } },
    { TAG_CAPTUREDTIME, { EXIF_TAG_DATE_TIME_ORIGINAL, OR_IFD_EXIF, &translateDate } },
    { TAG_CAPTUREDTIME, { EXIF_TAG_DATE_TIME_DIGITIZED, OR_IFD_EXIF, &translateDate } }
};

std::vector<IfdEntry::Ref> translateRecordEntry(const RecordEntry& e, Heap& heap, CiffIfd& ifd)
{
    std::vector<IfdEntry::Ref> vec;
    if (e.isHeap()) {
        const CIFFContainer* ciffc = dynamic_cast<const CIFFContainer*>(&ifd.container());
        LOGASSERT(ciffc);
        Heap h = e.heap(heap, ciffc);
        for (auto rec : h.records()) {
            auto r = translateRecordEntry(rec, h, ifd);
            vec.insert(vec.begin(), r.begin(), r.end());
        }
        return vec;
    }
    auto iter = ciff_exif_map.find(TAGCODE_MASK & e.typeCode);
    if (iter != ciff_exif_map.end()) {
        do {
            if (iter->first != (TAGCODE_MASK & e.typeCode)) {
                break;
            }
            // printf("tag 0x%x mapped to 0x%x in %d\n", iter->first, iter->second.exifTag,
            //       iter->second.dest);
            if (iter->second.dest == ifd.type()) {
                if (iter->second.converter) {
                    vec.push_back(
                        iter->second.converter.value_ref()(e, heap, ifd, iter->second.exifTag));
                } else {
                    vec.push_back(
                        std::make_shared<IfdEntry>(
                            iter->second.exifTag, e.exifType(), e.count(), 0, ifd));
                }
            }
            iter++;
        } while(iter != ciff_exif_map.end());
    } else {
        // printf("No mapping for 0x%x\n", e.typeCode & TAGCODE_MASK);
    }

    return vec;
}

}

bool CiffExifIfd::load()
{
    auto container = static_cast<CIFFContainer*>(m_file.getContainer());
    HeapRef props = container->getImageProps();
    if (props) {
        const CIFF::RecordEntryList& propsRecs = props->records();
        for (const auto& rec : propsRecs) {
            auto ifdentries = translateRecordEntry(rec, *props, *this);
            if (!ifdentries.empty()) {
                for (auto entry2 : ifdentries) {
                    m_entries[entry2->id()] = entry2;
                }
            }
        }

        auto iter = std::find_if(propsRecs.cbegin(), propsRecs.cend(),
                                 [] (const auto& e) {
                                     return e.isA(TAG_EXIFINFORMATION);
                                 });
        if (iter != propsRecs.end()) {
            Heap exifProps = iter->heap(*props, container);

            for (auto rec : exifProps.records()) {
                auto ifdentries = translateRecordEntry(rec, exifProps, *this);
                if (!ifdentries.empty()) {
                    for (auto entry2 : ifdentries) {
                        m_entries[entry2->id()] = entry2;
                    }
                }
            }
        }
    }
    return true;
}

}
}
}
