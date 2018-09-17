/*
 * libopenraw - missingcoefficients.cpp
 *
 * Copyright (C) 2018 Hubert Figuière
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


/* @brief a utility to list the missing coefficients */
/*
 * This code hooks to internals of libopenraw and is not to be interpreted
 * as a sample code. It is instead a helper used by developers to make sure
 * of the support of various cameras.
 */

#include <iostream>
#include <memory>

#include "../lib/rawfile.hpp"
#include "../lib/rawfile_private.hpp"
#include "../lib/arwfile.hpp"
#include "../lib/cr2file.hpp"
#include "../lib/cr3file.hpp"
#include "../lib/crwfile.hpp"
#include "../lib/erffile.hpp"
#include "../lib/mrwfile.hpp"
#include "../lib/neffile.hpp"
#include "../lib/orffile.hpp"
#include "../lib/peffile.hpp"
#include "../lib/raffile.hpp"
#include "../lib/rw2file.hpp"
#include "../lib/io/memstream.hpp"

using namespace OpenRaw;
using namespace OpenRaw::Internals;


namespace OpenRaw {
namespace Internals {

template<typename T>
void audit_coefficients()
{
  IO::Stream::Ptr s = std::make_shared<IO::MemStream>(nullptr, 0);
  T t(s);
  const BuiltinColourMatrix* matrices = t._getMatrices();
  const RawFile::camera_ids_t* def = T::s_def;
  for (auto current = def; current->model; current++) {
    bool found = false;
    for (auto current_mat = matrices; current_mat->camera != 0; current_mat++) {
      if (current_mat->camera == current->type_id) {
        found = true;
        break;
      }
    }
    if (!found) {
      std::cout << "Missing coefficient for " << current->model << std::endl;
    }
  }
}

}
}


int main(int, char**)
{
  OpenRaw::Internals::audit_coefficients<ArwFile>();
  OpenRaw::Internals::audit_coefficients<Cr2File>();
  OpenRaw::Internals::audit_coefficients<Cr3File>();
  OpenRaw::Internals::audit_coefficients<CRWFile>();
  OpenRaw::Internals::audit_coefficients<ERFFile>();
  OpenRaw::Internals::audit_coefficients<MRWFile>();
  OpenRaw::Internals::audit_coefficients<NefFile>();
  OpenRaw::Internals::audit_coefficients<OrfFile>();
  OpenRaw::Internals::audit_coefficients<PEFFile>();
  OpenRaw::Internals::audit_coefficients<RafFile>();
  OpenRaw::Internals::audit_coefficients<Rw2File>();
}
