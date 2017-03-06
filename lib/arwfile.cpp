/*
 * libopenraw - arwfile.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuiere
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


#include <libopenraw/cameraids.h>

#include "ifdfilecontainer.hpp"
#include "arwfile.hpp"
#include "rawfile_private.hpp"

using namespace Debug;

namespace OpenRaw {

class RawData;

namespace Internals {

#define OR_MAKE_SONY_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_SONY,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A100), 0, 0xfeb,
      { 9437,-2811,-774,-8405,16215,2290,-710,596,7181 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A200), 0, 0,
      { 9847,-3091,-928,-8485,16345,2225,-715,595,7103 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A380), 0, 0,
      { 6038,-1484,-579,-9145,16746,2512,-875,746,7218 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A390), 0, 0,
      { 6038,-1484,-579,-9145,16746,2512,-875,746,7218 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A550), 128, 0xfeb,
      { 4950,-580,-103,-5228,12542,3029,-709,1435,7371 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A560), 128, 0xfeb,
      { 4950,-580,-103,-5228,12542,3029,-709,1435,7371 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A700), 126, 0,
      { 5775,-805,-359,-8574,16295,2391,-1943,2341,7249 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A850), 128, 0,
      { 5413,-1162,-365,-5665,13098,2866,-608,1179,8440 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A900), 128, 0,
      { 5209,-1072,-397,-8845,16120,2919,-1618,1803,8654 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA33), 128, 0,
      { 6069,-1221,-366,-5221,12779,2734,-1024,2066,6834 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA35), 128, 0,
      { 5986,-1618,-415,-4557,11820,3120,-681,1404,6971 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA55), 128, 0,
      { 5932,-1492,-411,-4813,12285,2856,-741,1524,6739 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA57), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA58), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA65), 128, 0,
      { 5491,-1192,-363,-4951,12342,2948,-911,1722,7192 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA77), 128, 0,
      { 5491,-1192,-363,-4951,12342,2948,-911,1722,7192 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA99),
      0,
      0,
      { 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX3), 128, 0,	/* Adobe */
      { 6549,-1550,-436,-4880,12435,2753,-854,1868,6976 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5), 128, 0,	/* Adobe */
      { 6549,-1550,-436,-4880,12435,2753,-854,1868,6976 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5N), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5R), 128, 0,
      { 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5T), 128, 0,
      { 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEXC3), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEXF3), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX6), 128, 0,
      { 6129,-1545,-418,-4930,12490,2743,-977,1693,6615 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX7), 128, 0,
      { 5491,-1192,-363,-4951,12342,2948,-911,1722,7192 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100),
      0,
      0,
      { 8651, -2754, -1057, -3464, 12207, 1373, -568, 1398, 4434 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M2),
      0,
      0,
      { 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M3),
      0,
      0,
      { 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M4),
      0,
      0,
      { 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX1),
      0,
      0,
      { 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX1R),
      0,
      0,
      { 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX1RM2),
      0,
      0,
      { 6629, -1900, -483, -4618, 12349, 2550, -622, 1381, 6514 } },

    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A3000), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A6000), 128, 0,
      { 5991,-1456,-455,-4764,12135,2980,-707,1425,6701 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A6300),
      0,
      0,
      { 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A7), 128, 0,
      { 5271,-712,-347,-6153,13653,2763,-1601,2366,7242 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A7R), 128, 0,
      { 4913,-541,-202,-6130,13513,2906,-1564,2151,7183 } },
    { OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A7RM2),
      0,
      0,
      { 6629, -1900, -483, -4618, 12349, 2550, -622, 1381, 6514 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t ArwFile::s_def[] = {
    { "DSLR-A100", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A100) },
    { "DSLR-A200", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A200) },
    { "DSLR-A380", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A380) },
    { "DSLR-A390", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A390) },
    { "DSLR-A550", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A550) },
    { "DSLR-A560", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A560) },
    { "DSLR-A580", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A580) },
    { "DSLR-A700", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A700) },
    { "DSLR-A850", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A850) },
    { "DSLR-A900", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A900) },
    { "SLT-A33", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA33) },
    // Likely a pre-release.
    { "SLT-A00", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA35) },
    { "SLT-A55V", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA55) },
    { "SLT-A57", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA57) },
    { "SLT-A58", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA58) },
    { "SLT-A65V", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA65) },
    { "SLT-A77V", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA77) },
    { "SLT-A99V", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_SLTA99) },
    { "NEX-3", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX3) },
    { "NEX-5", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5) },
    { "NEX-5N", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5N) },
    // There are pre-production files with the type NEX-C00....
    { "NEX-C3", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEXC3) },
    { "NEX-F3", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEXF3) },
    { "NEX-7", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX7) },
    { "DSC-RX10", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX10) },
    { "DSC-RX10M2", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX10M2) },
    { "DSC-RX10M3", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX10M3) },
    { "DSC-RX100", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100) },
    { "DSC-RX100M2", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M2) },
    { "DSC-RX100M3", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M3) },
    { "DSC-RX100M4", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M4) },
    { "DSC-RX100M5", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX100M5) },
    { "DSC-RX1", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX1) },
    { "DSC-RX1R", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX1R) },
    { "DSC-RX1RM2", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_RX1RM2) },
    { "NEX-6", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX6) },
    { "NEX-5R", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5R) },
    { "NEX-5T", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_NEX5T) },
    { "ILCA-99M2", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A99M2) },
    { "ILCE-3000", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A3000) },
    { "ILCE-6000", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A6000) },
    { "ILCE-6300", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A6300) },
    { "ILCE-6500", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A6500) },
    { "ILCE-7", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A7) },
    { "ILCE-7R", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A7R) },
    { "ILCE-7RM2", OR_MAKE_SONY_TYPEID(OR_TYPEID_SONY_A7RM2) },
    { 0, 0 }
};


RawFile *ArwFile::factory(const IO::Stream::Ptr & s)
{
    return new ArwFile(s);
}

ArwFile::ArwFile(const IO::Stream::Ptr &s)
    : TiffEpFile(s, OR_RAWFILE_TYPE_ARW)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

ArwFile::~ArwFile()
{
}

IfdDir::Ref  ArwFile::_locateCfaIfd()
{
    if(!isA100())
    {
        return TiffEpFile::_locateCfaIfd();
    }
    return mainIfd();
}


IfdDir::Ref  ArwFile::_locateMainIfd()
{
    return m_container->setDirectory(0);
}

::or_error ArwFile::_getRawData(RawData & data, uint32_t options)
{
    if(isA100())
    {
        // TODO implement for A100
        return OR_ERROR_NOT_FOUND;
    }
    return TiffEpFile::_getRawData(data, options);
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
