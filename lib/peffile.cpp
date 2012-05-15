/*
 * libopenraw - peffile.cpp
 *
 * Copyright (C) 2006-2008, 2010, 2012 Hubert Figuiere
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


#include <iostream>
#include <libopenraw/cameraids.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "io/file.h"
#include "peffile.h"
#include "rawfile_private.h"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_PENTAX_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_D), 0, 0,
	{ 9651,-2059,-1189,-8881,16512,2487,-1460,1345,10687 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL), 0, 0,
	{ 10829,-2838,-1115,-8339,15817,2696,-837,680,11939 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K10D_PEF), 0, 0,
	{ 9566,-2863,-803,-7170,15172,2112,-818,803,9705 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF), 0, 0,
	{ 11095,-3157,-1324,-8377,15834,2720,-1108,947,11688 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF), 0, 0,
	{ 11095,-3157,-1324,-8377,15834,2720,-1108,947,11688 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K20D_PEF), 0, 0,
	{ 9427,-2714,-868,-7493,16092,1373,-2199,3264,7180 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KR_PEF), 0, 0,
	{ 9895,-3077,-850,-5304,13035,2521,-883,1768,6936 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_PEF), 0, 0,
	{ 8713,-2833,-743,-4342,11900,2772,-722,1543,6247 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K7_PEF), 0, 0,
	{ 9142,-2947,-678,-8648,16967,1663,-2224,2898,8615 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KX_PEF), 0, 0,
	{ 8843,-2837,-625,-5025,12644,2668,-411,1234,7410 } },
    { OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_645D_PEF), 0, 0x3e00,
	{ 10646,-3593,-1158,-3329,11699,1831,-667,2874,6287 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const struct IfdFile::camera_ids_t PEFFile::s_def[] = {
    { "PENTAX *ist D      ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_D) },
    { "PENTAX *ist DL     ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_IST_DL) },
    { "PENTAX K10D        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K10D_PEF) },
    { "PENTAX K100D       ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF) },
    { "PENTAX K100D Super ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K100D_PEF) },
    { "PENTAX K20D        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K20D_PEF) },
    { "PENTAX K-r         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KR_PEF) },
    { "PENTAX K-5         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K5_PEF) },
    { "PENTAX K-7         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_K7_PEF) },
    { "PENTAX K-x         ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_KX_PEF) },
    { "PENTAX 645D        ", OR_MAKE_PENTAX_TYPEID(OR_TYPEID_PENTAX_645D_PEF) },
    { 0, 0 }
};


RawFile *PEFFile::factory(IO::Stream *s)
{
    return new PEFFile(s);
}

PEFFile::PEFFile(IO::Stream *s)
    : IfdFile(s, OR_RAWFILE_TYPE_PEF)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

PEFFile::~PEFFile()
{
}

IfdDir::Ref  PEFFile::_locateCfaIfd()
{
    // in PEF the CFA IFD is the main IFD
    return mainIfd();
}

IfdDir::Ref  PEFFile::_locateMainIfd()
{
    return m_container->setDirectory(0);
}

::or_error PEFFile::_getRawData(RawData & data, uint32_t options)
{
    ::or_error err;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    err = _getRawDataFromDir(data, _cfaIfd);
    if(err == OR_ERROR_NONE) {
        uint16_t compression = data.compression();
        switch(compression) {
        case IFD::COMPRESS_CUSTOM:
            if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
                // TODO decompress
            }
            break;
        default:
            break;
        }
    }
    return err;
}

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

