/*
 * libopenraw - dumputils.cpp
 *
 * Copyright (C) 2007-2020 Hubert Figuière
 * Copyright (C) 2008 Novell, Inc.
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

#include <boost/format.hpp>

#include "dumputils.hpp"

std::string typeToString(or_rawfile_type t)
{
    switch(t) {
    case OR_RAWFILE_TYPE_UNKNOWN:
        break;
    case OR_RAWFILE_TYPE_CR2:
        return "Canon CR2";
        break;
    case OR_RAWFILE_TYPE_CR3:
        return "Canon CR3";
        break;
    case OR_RAWFILE_TYPE_CRW:
        return "Canon CRW";
        break;
    case OR_RAWFILE_TYPE_NEF:
        return "Nikon NEF";
        break;
    case OR_RAWFILE_TYPE_NRW:
        return "Nikon NRW";
        break;
    case OR_RAWFILE_TYPE_MRW:
        return "Minolta MRW";
        break;
    case OR_RAWFILE_TYPE_ARW:
        return "Sony ARW";
        break;
    case OR_RAWFILE_TYPE_SR2:
        return "Sony SR2";
        break;
    case OR_RAWFILE_TYPE_DNG:
        return "Adobe DNG";
        break;
    case OR_RAWFILE_TYPE_ORF:
        return "Olympus ORF";
        break;
    case OR_RAWFILE_TYPE_PEF:
        return "Pentax PEF";
        break;
    case OR_RAWFILE_TYPE_ERF:
        return "Epson ERF";
        break;
    case OR_RAWFILE_TYPE_RW2:
        return "Panasonic RAW";
        break;
    case OR_RAWFILE_TYPE_RAF:
        return "FujiFilm RAF";
        break;
    case OR_RAWFILE_TYPE_TIFF:
        return "TIFF";
        break;
    case OR_RAWFILE_TYPE_GPR:
        return "GoPro GPR";
        break;
    }
    return "Unknown";
}


void dump_file_info(std::ostream& out, ORRawFileRef rf, bool dev_mode)
{
    or_rawfile_type fileType = or_rawfile_get_type(rf);
    out << boost::format("\tType = %1% (%2%)\n")
        % fileType % typeToString(fileType);
    or_rawfile_typeid fileTypeId = or_rawfile_get_typeid(rf);
    std::string typeId
        = str(boost::format("%1%, %2%")
              % OR_GET_FILE_TYPEID_VENDOR(fileTypeId)
              % OR_GET_FILE_TYPEID_CAMERA(fileTypeId));
    out << boost::format("\tType ID = %1%\n") % typeId;
    or_rawfile_typeid vendorId = or_rawfile_get_vendorid(rf);
    if (fileType == OR_RAWFILE_TYPE_DNG) {
        ORConstMetaValueRef original_value
            = or_rawfile_get_metavalue(rf, "Exif.Image.OriginalRawFileName");
        if (original_value) {
            auto original = or_metavalue_get_string(original_value);
            if (original != nullptr) {
                out << boost::format("\tConverted to DNG from '%1%'\n") % original;
            }
        }
    }
    if (vendorId != OR_GET_FILE_TYPEID_VENDOR(fileTypeId)) {
        out <<
            boost::format(
                "\t*ERROR*: mismatched vendor id, got %1%\n")
            % vendorId;
    }

    ORConstMetaValueRef make
        = or_rawfile_get_metavalue(rf, "Exif.Image.Make");
    if (make) {
        out << boost::format(dev_mode ?
                             "\tMake = \"%1%\"\n" : "\tMake = %1%\n")
            % or_metavalue_get_string(make);
    }
    ORConstMetaValueRef model
        = or_rawfile_get_metavalue(rf, "Exif.Image.Model");
    if (model) {
        out << boost::format(dev_mode ?
                               "\tModel = \"%1%\"\n" : "\tModel = %1%\n")
            % or_metavalue_get_string(model);
    }
    ORConstMetaValueRef uniqueCameraModel
        = or_rawfile_get_metavalue(rf, "Exif.Image.UniqueCameraModel");
    if (uniqueCameraModel) {
        out << boost::format("\tUnique Camera Model = %1%\n")
            % or_metavalue_get_string(uniqueCameraModel);
    }
}
